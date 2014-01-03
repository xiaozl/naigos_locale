/***********************************************************************
 *
 * CONFIG.C - Nagios Configuration CGI (View Only)
 *
 * Copyright (c) 1999-2009 Ethan Galstad (egalstad@nagios.org)
 * Last Modified: 05-15-2009
 *
 * This CGI program will display various configuration information.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 ***********************************************************************/

#include "../include/config.h"
#include "../include/common.h"
#include "../include/objects.h"
#include "../include/macros.h"
#include "../include/cgiutils.h"
#include "../include/cgiauth.h"
#include "../include/getcgi.h"

#include <stdio.h>
#include <locale.h>
#include <libintl.h>

#define _(string) gettext(string)

static nagios_macros *mac;

extern char   main_config_file[MAX_FILENAME_LENGTH];
extern char   url_html_path[MAX_FILENAME_LENGTH];
extern char   url_docs_path[MAX_FILENAME_LENGTH];
extern char   url_images_path[MAX_FILENAME_LENGTH];
extern char   url_logo_images_path[MAX_FILENAME_LENGTH];
extern char   url_stylesheets_path[MAX_FILENAME_LENGTH];

extern host *host_list;
extern service *service_list;
extern hostgroup *hostgroup_list;
extern servicegroup *servicegroup_list;
extern contactgroup *contactgroup_list;
extern command *command_list;
extern timeperiod *timeperiod_list;
extern contact *contact_list;
extern servicedependency *servicedependency_list;
extern serviceescalation *serviceescalation_list;
extern hostdependency *hostdependency_list;
extern hostescalation *hostescalation_list;


#define DISPLAY_NONE                     0
#define DISPLAY_HOSTS                    1
#define DISPLAY_HOSTGROUPS               2
#define DISPLAY_CONTACTS                 3
#define DISPLAY_CONTACTGROUPS            4
#define DISPLAY_SERVICES                 5
#define DISPLAY_TIMEPERIODS              6
#define DISPLAY_COMMANDS                 7
#define DISPLAY_HOSTGROUPESCALATIONS     8    /* no longer implemented */
#define DISPLAY_SERVICEDEPENDENCIES      9
#define DISPLAY_SERVICEESCALATIONS       10
#define DISPLAY_HOSTDEPENDENCIES         11
#define DISPLAY_HOSTESCALATIONS          12
#define DISPLAY_SERVICEGROUPS            15
#define DISPLAY_COMMAND_EXPANSION        16211

void document_header(int);
void document_footer(void);
int process_cgivars(void);

void display_options(void);

void display_hosts(void);
void display_hostgroups(void);
void display_servicegroups(void);
void display_contacts(void);
void display_contactgroups(void);
void display_services(void);
void display_timeperiods(void);
void display_commands(void);
void display_servicedependencies(void);
void display_serviceescalations(void);
void display_hostdependencies(void);
void display_hostescalations(void);
void display_command_expansion(void);

void unauthorized_message(void);


authdata current_authdata;

int display_type = DISPLAY_NONE;
char to_expand[MAX_COMMAND_BUFFER];
char hashed_color[8];

int embedded = FALSE;

void print_expand_input(int type) {
	char *seldesc = "";

	if(type == DISPLAY_COMMAND_EXPANSION) return;	/* Has its own form, w/ larger <input> */
	else if(type == DISPLAY_SERVICES) {
		seldesc = " Services Named or on Host";
		}
	else if(type == DISPLAY_SERVICEDEPENDENCIES) {
		seldesc = " Dependencies with Host";
		}
	else if(type == DISPLAY_SERVICEESCALATIONS) {
		seldesc = " Escalations on Host";
		}
	else if(type == DISPLAY_HOSTDEPENDENCIES) {
		seldesc = " Dependencies on/of Host";
		}
	else if(type == DISPLAY_HOSTESCALATIONS) {
		seldesc = " Escalations for Host";
		}
	printf("<tr><td align=left class='reportSelectSubTitle'>Show Only%s:</td></tr>\n", seldesc);
	printf("<tr><td align=left class='reportSelectItem'><input type='text' name='expand'\n");
	printf("value='%s'>", html_encode(to_expand, FALSE));
	}

int main(void) {
	int result = OK;
	mac = get_global_macros();

	/* get the arguments passed in the URL */
	process_cgivars();

	/* reset internal variables */
	reset_cgi_vars();

	/* read the CGI configuration file */
	result = read_cgi_config_file(get_cgi_config_location());
	if(result == ERROR) {
		document_header(FALSE);
		cgi_config_file_error(get_cgi_config_location());
		document_footer();
		return ERROR;
		}

	/* read the main configuration file */
	result = read_main_config_file(main_config_file);
	if(result == ERROR) {
		document_header(FALSE);
		main_config_file_error(main_config_file);
		document_footer();
		return ERROR;
		}

	/* read all object configuration data */
	result = read_all_object_configuration_data(main_config_file, READ_ALL_OBJECT_DATA);
	if(result == ERROR) {
		document_header(FALSE);
		object_data_error();
		document_footer();
		return ERROR;
		}

	/* initialize macros */
	init_macros();

	document_header(TRUE);

	/* get authentication information */
	get_authentication_information(&current_authdata);

	/* begin top table */
	printf("<table border=0 width=100%%>\n");
	printf("<tr>\n");

	/* left column of the first row */
	printf("<td align=left valign=top width=50%%>\n");
	display_info_table("Configuration", FALSE, &current_authdata);
	printf("</td>\n");

	/* right hand column of top row */
	printf("<td align=right valign=bottom width=50%%>\n");

	if(display_type != DISPLAY_NONE) {

		printf("<form method=\"get\" action=\"%s\">\n", CONFIG_CGI);
		printf("<table border=0>\n");

		printf("<tr><td align=left class='reportSelectSubTitle'>%s:</td></tr>\n",_("Object Type"));
		printf("<tr><td align=left class='reportSelectItem'>");
		printf("<select name='type'>\n");
		printf("<option value='hosts' %s>%s\n", (display_type == DISPLAY_HOSTS) ? "SELECTED" : "",_("Hosts"));
		printf("<option value='hostdependencies' %s>%s\n", (display_type == DISPLAY_HOSTDEPENDENCIES) ? "SELECTED" : "",_("Host Dependencies"));
		printf("<option value='hostescalations' %s>%s\n", (display_type == DISPLAY_HOSTESCALATIONS) ? "SELECTED" : "",_("Host Escalations"));
		printf("<option value='hostgroups' %s>%s\n", (display_type == DISPLAY_HOSTGROUPS) ? "SELECTED" : "",_("Host Groups"));
		printf("<option value='services' %s>%s\n", (display_type == DISPLAY_SERVICES) ? "SELECTED" : "",_("Services"));
		printf("<option value='servicegroups' %s>%s\n", (display_type == DISPLAY_SERVICEGROUPS) ? "SELECTED" : "",_("Service Groups"));
		printf("<option value='servicedependencies' %s>%s\n", (display_type == DISPLAY_SERVICEDEPENDENCIES) ? "SELECTED" : "",_("Service Dependencies"));
		printf("<option value='serviceescalations' %s>%s\n", (display_type == DISPLAY_SERVICEESCALATIONS) ? "SELECTED" : "",_("Service Dependencies"));
		printf("<option value='contacts' %s>%s\n", (display_type == DISPLAY_CONTACTS) ? "SELECTED" : "",_("Contacts"));
		printf("<option value='contactgroups' %s>%s\n", (display_type == DISPLAY_CONTACTGROUPS) ? "SELECTED" : "",_("Contact Groups"));
		printf("<option value='timeperiods' %s>%s\n", (display_type == DISPLAY_TIMEPERIODS) ? "SELECTED" : "",_("Timeperiods"));
		printf("<option value='commands' %s>%s\n", (display_type == DISPLAY_COMMANDS) ? "SELECTED" : "",_("Commands"));
		printf("<option value='command' %s>%s\n", (display_type == DISPLAY_COMMAND_EXPANSION) ? "SELECTED" : "",_("Command Expansion"));
		printf("</select>\n");
		printf("</td></tr>\n");

		print_expand_input(display_type);

		printf("<tr><td class='reportSelectItem'><input type='submit' value='%s'></td></tr>\n",_("Update"));
		printf("</table>\n");
		printf("</form>\n");
		}

	/* display context-sensitive help */
	switch(display_type) {
		case DISPLAY_HOSTS:
			display_context_help(CONTEXTHELP_CONFIG_HOSTS);
			break;
		case DISPLAY_HOSTGROUPS:
			display_context_help(CONTEXTHELP_CONFIG_HOSTGROUPS);
			break;
		case DISPLAY_SERVICEGROUPS:
			display_context_help(CONTEXTHELP_CONFIG_SERVICEGROUPS);
			break;
		case DISPLAY_CONTACTS:
			display_context_help(CONTEXTHELP_CONFIG_CONTACTS);
			break;
		case DISPLAY_CONTACTGROUPS:
			display_context_help(CONTEXTHELP_CONFIG_CONTACTGROUPS);
			break;
		case DISPLAY_SERVICES:
			display_context_help(CONTEXTHELP_CONFIG_SERVICES);
			break;
		case DISPLAY_TIMEPERIODS:
			display_context_help(CONTEXTHELP_CONFIG_TIMEPERIODS);
			break;
		case DISPLAY_COMMANDS:
			display_context_help(CONTEXTHELP_CONFIG_COMMANDS);
			break;
		case DISPLAY_SERVICEDEPENDENCIES:
			display_context_help(CONTEXTHELP_CONFIG_SERVICEDEPENDENCIES);
			break;
		case DISPLAY_SERVICEESCALATIONS:
			display_context_help(CONTEXTHELP_CONFIG_HOSTESCALATIONS);
			break;
		case DISPLAY_HOSTDEPENDENCIES:
			display_context_help(CONTEXTHELP_CONFIG_HOSTDEPENDENCIES);
			break;
		case DISPLAY_HOSTESCALATIONS:
			display_context_help(CONTEXTHELP_CONFIG_HOSTESCALATIONS);
			break;
		case DISPLAY_COMMAND_EXPANSION:
			/* Reusing DISPLAY_COMMANDS help until further notice */
			display_context_help(CONTEXTHELP_CONFIG_COMMANDS);
			break;
		default:
			display_context_help(CONTEXTHELP_CONFIG_MENU);
			break;
		}

	printf("</td>\n");

	/* end of top table */
	printf("</tr>\n");
	printf("</table>\n");


	switch(display_type) {
		case DISPLAY_HOSTS:
			display_hosts();
			break;
		case DISPLAY_HOSTGROUPS:
			display_hostgroups();
			break;
		case DISPLAY_SERVICEGROUPS:
			display_servicegroups();
			break;
		case DISPLAY_CONTACTS:
			display_contacts();
			break;
		case DISPLAY_CONTACTGROUPS:
			display_contactgroups();
			break;
		case DISPLAY_SERVICES:
			display_services();
			break;
		case DISPLAY_TIMEPERIODS:
			display_timeperiods();
			break;
		case DISPLAY_COMMANDS:
			display_commands();
			break;
		case DISPLAY_SERVICEDEPENDENCIES:
			display_servicedependencies();
			break;
		case DISPLAY_SERVICEESCALATIONS:
			display_serviceescalations();
			break;
		case DISPLAY_HOSTDEPENDENCIES:
			display_hostdependencies();
			break;
		case DISPLAY_HOSTESCALATIONS:
			display_hostescalations();
			break;
		case DISPLAY_COMMAND_EXPANSION:
			display_command_expansion();
			break;
		default:
			display_options();
			break;
		}

	document_footer();

	return OK;
	}




void document_header(int use_stylesheet) {
	char date_time[MAX_DATETIME_LENGTH];
	time_t t;

	if(embedded == TRUE)
		return;

	time(&t);
	get_time_string(&t, date_time, sizeof(date_time), HTTP_DATE_TIME);

	printf("Cache-Control: no-store\r\n");
	printf("Pragma: no-cache\r\n");
	printf("Last-Modified: %s\r\n", date_time);
	printf("Expires: %s\r\n", date_time);
	printf("Content-type: text/html\r\n\r\n");

	printf("<html>\n");
	printf("<head>\n");
	printf("<link rel=\"shortcut icon\" href=\"%sfavicon.ico\" type=\"image/ico\">\n", url_images_path);
	printf("<META HTTP-EQUIV='Pragma' CONTENT='no-cache'>\n");
	printf("<meta http-equiv='content-type' content='text/html;charset=UTF-8'>\n");
	printf("<title>\n");
	printf("%s\n",_("Configuration"));
	printf("</title>\n");

	if(use_stylesheet == TRUE) {
		printf("<LINK REL='stylesheet' TYPE='text/css' HREF='%s%s'>\n", url_stylesheets_path, COMMON_CSS);
		printf("<LINK REL='stylesheet' TYPE='text/css' HREF='%s%s'>\n", url_stylesheets_path, CONFIG_CSS);
		}

	printf("</head>\n");

	printf("<body CLASS='config'>\n");

	/* include user SSI header */
	include_ssi_files(CONFIG_CGI, SSI_HEADER);

	return;
	}


void document_footer(void) {

	if(embedded == TRUE)
		return;

	/* include user SSI footer */
	include_ssi_files(CONFIG_CGI, SSI_FOOTER);

	printf("</body>\n");
	printf("</html>\n");

	return;
	}


int process_cgivars(void) {
	char **variables;
	int error = FALSE;
	int x;

	variables = getcgivars();
	to_expand[0] = '\0';

	for(x = 0; variables[x] != NULL; x++) {

		/* do some basic length checking on the variable identifier to prevent buffer overflows */
		if(strlen(variables[x]) >= MAX_INPUT_BUFFER - 1) {
			x++;
			continue;
			}

		/* we found the configuration type argument */
		else if(!strcmp(variables[x], "type")) {
			x++;
			if(variables[x] == NULL) {
				error = TRUE;
				break;
				}

			/* what information should we display? */
			if(!strcmp(variables[x], "hosts"))
				display_type = DISPLAY_HOSTS;
			else if(!strcmp(variables[x], "hostgroups"))
				display_type = DISPLAY_HOSTGROUPS;
			else if(!strcmp(variables[x], "servicegroups"))
				display_type = DISPLAY_SERVICEGROUPS;
			else if(!strcmp(variables[x], "contacts"))
				display_type = DISPLAY_CONTACTS;
			else if(!strcmp(variables[x], "contactgroups"))
				display_type = DISPLAY_CONTACTGROUPS;
			else if(!strcmp(variables[x], "services"))
				display_type = DISPLAY_SERVICES;
			else if(!strcmp(variables[x], "timeperiods"))
				display_type = DISPLAY_TIMEPERIODS;
			else if(!strcmp(variables[x], "commands"))
				display_type = DISPLAY_COMMANDS;
			else if(!strcmp(variables[x], "servicedependencies"))
				display_type = DISPLAY_SERVICEDEPENDENCIES;
			else if(!strcmp(variables[x], "serviceescalations"))
				display_type = DISPLAY_SERVICEESCALATIONS;
			else if(!strcmp(variables[x], "hostdependencies"))
				display_type = DISPLAY_HOSTDEPENDENCIES;
			else if(!strcmp(variables[x], "hostescalations"))
				display_type = DISPLAY_HOSTESCALATIONS;
			else if(!strcmp(variables[x], "command"))
				display_type = DISPLAY_COMMAND_EXPANSION;

			/* we found the embed option */
			else if(!strcmp(variables[x], "embedded"))
				embedded = TRUE;
			}

		/* we found the string-to-expand argument */
		else if(!strcmp(variables[x], "expand")) {
			x++;
			if(variables[x] == NULL) {
				error = TRUE;
				break;
				}
			strncpy(to_expand, variables[x], MAX_COMMAND_BUFFER);
			to_expand[MAX_COMMAND_BUFFER - 1] = '\0';
			}


		/* we received an invalid argument */
		else
			error = TRUE;

		}

	/* free memory allocated to the CGI variables */
	free_cgivars(variables);

	return error;
	}



void display_hosts(void) {
	host *temp_host = NULL;
	hostsmember *temp_hostsmember = NULL;
	contactsmember *temp_contactsmember = NULL;
	contactgroupsmember *temp_contactgroupsmember = NULL;
	char *processed_string = NULL;
	int options = 0;
	int odd = 0;
	char time_string[16];
	char *bg_class = "";
	int contact = 0;

	/* see if user is authorized to view host information... */
	if(is_authorized_for_configuration_information(&current_authdata) == FALSE) {
		unauthorized_message();
		return;
		}

	printf("<P><DIV ALIGN=CENTER CLASS='dataTitle'>%s%s%s</DIV></P>\n",_("Host"),
	       (*to_expand == '\0' ? "s" : " "), (*to_expand == '\0' ? "" : html_encode(to_expand, FALSE)));

	printf("<P><DIV ALIGN=CENTER>\n");
	printf("<TABLE BORDER=0 CLASS='data'>\n");

	printf("<TR>\n");
	printf("<TH CLASS='data'>%s</TH>",_("Host Name"));
	printf("<TH CLASS='data'>%s</TH>",_("Alias/Description"));
	printf("<TH CLASS='data'>%s</TH>",_("Address"));
	printf("<TH CLASS='data'>%s</TH>",_("Parent Hosts"));
	printf("<TH CLASS='data'>%s</TH>",_("Max. Check Attempts"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Check Interval"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Retry Interval"));
	printf("<TH CLASS='data'>%s</TH>",_("Host Check Command"));
	printf("<TH CLASS='data'>%s</TH>",_("Check Period"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Obsess Over"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Enable Active Checks"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Enable Passive Checks"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Check Freshness"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Freshness Threshold"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Default Contacts/Groups"));
	printf("<TH CLASS='data'>%s</TH>",_("Notification Interval"));
	printf("<TH CLASS='data'>%s</TH>",_("First Notification Delay"));
	printf("<TH CLASS='data'>%s</TH>",_("Notification Options"));
	printf("<TH CLASS='data'>%s</TH>",_("Notification Period"));
	printf("<TH CLASS='data'>%s</TH>",_("Event Handler"));
	printf("<TH CLASS='data'>%s</TH>",_("Enable Event Handler"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Stalking Options"));
	printf("<TH CLASS='data'>%s</TH>",_("Enable Flap Detection"));
	printf("<TH CLASS='data'>%s</TH>",_("Low Flap Threshold"));
	printf("<TH CLASS='data'>%s</TH>",_("High Flap Threshold"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Flap Detection Options"));
	printf("<TH CLASS='data'>%s</TH>",_("Process Performance Data"));
	printf("<TH CLASS='data'>%s</TH>",_("Enable Failure Prediction"));
	printf("<TH CLASS='data'>%s</TH>",_("Failure Prediction Options"));
	printf("<TH CLASS='data'>%s</TH>",_("Notes"));
	printf("<TH CLASS='data'>%s</TH>",_("Notes URL"));
	printf("<TH CLASS='data'>%s</TH>",_("Action URL"));
	printf("<TH CLASS='data'>%s</TH>",_("2-D Coords"));
	printf("<TH CLASS='data'>%s</TH>",_("3-D Coords"));
	printf("<TH CLASS='data'>%s</TH>",_("Statusmap Image"));
	printf("<TH CLASS='data'>%s</TH>",_("VRML Image"));
	printf("<TH CLASS='data'>%s</TH>",_("Logo Image"));
	printf("<TH CLASS='data'>%s</TH>",_("Image Alt"));
	printf("<TH CLASS='data'>%s</TH>",_("Retention Options"));
	printf("</TR>\n");

	/* check all the hosts... */
	for(temp_host = host_list; temp_host != NULL; temp_host = temp_host->next) if(((*to_expand) == '\0') || !strcmp(to_expand, temp_host->name)) {

			/* grab macros */
			grab_host_macros_r(mac, temp_host);

			if(odd) {
				odd = 0;
				bg_class = "dataOdd";
				}
			else {
				odd = 1;
				bg_class = "dataEven";
				}

			printf("<TR CLASS='%s'>\n", bg_class);

			printf("<TD CLASS='%s'><a name='%s'><a href='%s?type=services&expand=%s'>%s</a></a></TD>\n", bg_class,
			       url_encode(temp_host->name), CONFIG_CGI, url_encode(temp_host->name), html_encode(temp_host->name, FALSE));
			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, html_encode(temp_host->alias, FALSE));
			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, html_encode(temp_host->address, FALSE));

			printf("<TD CLASS='%s'>", bg_class);
			for(temp_hostsmember = temp_host->parent_hosts; temp_hostsmember != NULL; temp_hostsmember = temp_hostsmember->next) {

				if(temp_hostsmember != temp_host->parent_hosts)
					printf(", ");

				printf("<a href='%s?type=hosts&expand=%s'>%s</a>\n", CONFIG_CGI, url_encode(temp_hostsmember->host_name), html_encode(temp_hostsmember->host_name, FALSE));
				}
			if(temp_host->parent_hosts == NULL)
				printf("&nbsp;");
			printf("</TD>\n");

			printf("<TD CLASS='%s'>%d</TD>\n", bg_class, temp_host->max_attempts);

			get_interval_time_string(temp_host->check_interval, time_string, sizeof(time_string));
			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, time_string);

			get_interval_time_string(temp_host->retry_interval, time_string, sizeof(time_string));
			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, time_string);

			printf("<TD CLASS='%s'>", bg_class);
			if(temp_host->host_check_command == NULL)
				printf("&nbsp;");
			else
				/* printf("<a href='%s?type=commands&expand=%s'>%s</a></TD>\n",CONFIG_CGI,url_encode(strtok(temp_host->host_check_command,"!")),html_encode(temp_host->host_check_command,FALSE)); */
				printf("<a href='%s?type=command&expand=%s'>%s</a></TD>\n", CONFIG_CGI, url_encode(temp_host->host_check_command), html_encode(temp_host->host_check_command, FALSE));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			if(temp_host->check_period == NULL)
				printf("&nbsp;");
			else
				printf("<A HREF='%s?type=timeperiods&expand=%s'>%s</A>", CONFIG_CGI, url_encode(temp_host->check_period), html_encode(temp_host->check_period, FALSE));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, (temp_host->obsess_over_host == TRUE) ? "Yes" : "No");

			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, (temp_host->checks_enabled == TRUE) ? "Yes" : "No");

			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, (temp_host->accept_passive_host_checks == TRUE) ? "Yes" : "No");

			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, (temp_host->check_freshness == TRUE) ? "Yes" : "No");

			printf("<TD CLASS='%s'>", bg_class);
			if(temp_host->freshness_threshold == 0)
				printf("%s\n",_("Auto-determined value"));
			else
				printf("%d %s\n", temp_host->freshness_threshold,_("seconds"));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);

			/* find all the contacts for this host... */
			contact = 0;
			for(temp_contactsmember = temp_host->contacts; temp_contactsmember != NULL; temp_contactsmember = temp_contactsmember->next) {
				contact++;
				if(contact > 1)
					printf(", ");

				printf("<A HREF='%s?type=contacts&expand=%s'>%s</A>\n", CONFIG_CGI, url_encode(temp_contactsmember->contact_name), html_encode(temp_contactsmember->contact_name, FALSE));
				}
			for(temp_contactgroupsmember = temp_host->contact_groups; temp_contactgroupsmember != NULL; temp_contactgroupsmember = temp_contactgroupsmember->next) {
				contact++;
				if(contact > 1)
					printf(", ");
				printf("<A HREF='%s?type=contactgroups&expand=%s'>%s</A>\n", CONFIG_CGI, url_encode(temp_contactgroupsmember->group_name), html_encode(temp_contactgroupsmember->group_name, FALSE));
				}
			if(contact == 0)
				printf("&nbsp;");
			printf("</TD>\n");

			get_interval_time_string(temp_host->notification_interval, time_string, sizeof(time_string));
			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, (temp_host->notification_interval == 0) ? _("<i>No Re-notification</I>") : html_encode(time_string, FALSE));

			get_interval_time_string(temp_host->first_notification_delay, time_string, sizeof(time_string));
			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, time_string);

			printf("<TD CLASS='%s'>", bg_class);
			options = 0;
			if(temp_host->notify_on_down == TRUE) {
				options = 1;
				printf("%s",_("Down"));
				}
			if(temp_host->notify_on_unreachable == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Unreachable"));
				options = 1;
				}
			if(temp_host->notify_on_recovery == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Recovery"));
				options = 1;
				}
			if(temp_host->notify_on_flapping == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Flapping"));
				options = 1;
				}
			if(temp_host->notify_on_downtime == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Downtime"));
				options = 1;
				}
			if(options == 0)
				printf("%s",_("None"));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			if(temp_host->notification_period == NULL)
				printf("&nbsp;");
			else
				printf("<a href='%s?type=timeperiods&expand=%s'>%s</a>", CONFIG_CGI, url_encode(temp_host->notification_period), html_encode(temp_host->notification_period, FALSE));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			if(temp_host->event_handler == NULL)
				printf("&nbsp");
			else
				/* printf("<a href='%s?type=commands&expand=%s'>%s</a></TD>\n",CONFIG_CGI,url_encode(strtok(temp_host->event_handler,"!")),html_encode(temp_host->event_handler,FALSE)); */
				printf("<a href='%s?type=command&expand=%s'>%s</a></TD>\n", CONFIG_CGI, url_encode(temp_host->event_handler), html_encode(temp_host->event_handler, FALSE));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			printf("%s\n", (temp_host->event_handler_enabled == TRUE) ? _("Yes") : _("No"));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			options = 0;
			if(temp_host->stalk_on_up == TRUE) {
				options = 1;
				printf("%s",_("Up"));
				}
			if(temp_host->stalk_on_down == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Up"));
				options = 1;
				}
			if(temp_host->stalk_on_unreachable == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Unreachable"));
				options = 1;
				}
			if(options == 0)
				printf("%s",_("None"));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			printf("%s\n", (temp_host->flap_detection_enabled == TRUE) ? _("Yes") : _("No"));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			if(temp_host->low_flap_threshold == 0.0)
				printf("%s\n",_("None"));
			else
				printf("%3.1f%%\n", temp_host->low_flap_threshold);
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			if(temp_host->high_flap_threshold == 0.0)
				printf("%s\n",_("Program-wide value"));
			else
				printf("%3.1f%%\n", temp_host->high_flap_threshold);
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			options = 0;
			if(temp_host->flap_detection_on_up == TRUE) {
				options = 1;
				printf("%s",_("Up"));
				}
			if(temp_host->flap_detection_on_down == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Up"));
				options = 1;
				}
			if(temp_host->flap_detection_on_unreachable == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Unreachable"));
				options = 1;
				}
			if(options == 0)
				printf("%s",_("Unreachable"));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			printf("%s\n", (temp_host->process_performance_data == TRUE) ? _("Yes") : _("No"));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			printf("%s\n", (temp_host->failure_prediction_enabled == TRUE) ? _("Yes") : _("No"));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, (temp_host->failure_prediction_options == NULL) ? "&nbsp;" : html_encode(temp_host->failure_prediction_options, FALSE));

			printf("<TD CLASS='%s'>%s</TD>", bg_class, (temp_host->notes == NULL) ? "&nbsp;" : html_encode(temp_host->notes, FALSE));

			printf("<TD CLASS='%s'>%s</TD>", bg_class, (temp_host->notes_url == NULL) ? "&nbsp;" : html_encode(temp_host->notes_url, FALSE));

			printf("<TD CLASS='%s'>%s</TD>", bg_class, (temp_host->action_url == NULL) ? "&nbsp;" : html_encode(temp_host->action_url, FALSE));

			if(temp_host->have_2d_coords == FALSE)
				printf("<TD CLASS='%s'>&nbsp;</TD>", bg_class);
			else
				printf("<TD CLASS='%s'>%d,%d</TD>", bg_class, temp_host->x_2d, temp_host->y_2d);

			if(temp_host->have_3d_coords == FALSE)
				printf("<TD CLASS='%s'>&nbsp;</TD>", bg_class);
			else
				printf("<TD CLASS='%s'>%.2f,%.2f,%.2f</TD>", bg_class, temp_host->x_3d, temp_host->y_3d, temp_host->z_3d);

			if(temp_host->statusmap_image == NULL)
				printf("<TD CLASS='%s'>&nbsp;</TD>", bg_class);
			else
				printf("<TD CLASS='%s' valign='center'><img src='%s%s' border='0' width='20' height='20'> %s</TD>", bg_class, url_logo_images_path, temp_host->statusmap_image, html_encode(temp_host->statusmap_image, FALSE));

			if(temp_host->vrml_image == NULL)
				printf("<TD CLASS='%s'>&nbsp;</TD>", bg_class);
			else
				printf("<TD CLASS='%s' valign='center'><img src='%s%s' border='0' width='20' height='20'> %s</TD>", bg_class, url_logo_images_path, temp_host->vrml_image, html_encode(temp_host->vrml_image, FALSE));

			if(temp_host->icon_image == NULL)
				printf("<TD CLASS='%s'>&nbsp;</TD>", bg_class);
			else {
				process_macros_r(mac, temp_host->icon_image, &processed_string, 0);
				printf("<TD CLASS='%s' valign='center'><img src='%s%s' border='0' width='20' height='20'> %s</TD>", bg_class, url_logo_images_path, processed_string, html_encode(temp_host->icon_image, FALSE));
				free(processed_string);
				}

			printf("<TD CLASS='%s'>%s</TD>", bg_class, (temp_host->icon_image_alt == NULL) ? "&nbsp;" : html_encode(temp_host->icon_image_alt, FALSE));

			printf("<TD CLASS='%s'>", bg_class);
			options = 0;
			if(temp_host->retain_status_information == TRUE) {
				options = 1;
				printf("%s",_("Unreachable"));
				}
			if(temp_host->retain_nonstatus_information == TRUE) {
				printf("%s%s", (options == 1) ? ", " : "",_("Non-Status Information"));
				options = 1;
				}
			if(options == 0)
				printf("%s",_("None"));
			printf("</TD>\n");

			printf("</TR>\n");
			}

	printf("</TABLE>\n");
	printf("</DIV>\n");
	printf("</P>\n");

	return;
	}



void display_hostgroups(void) {
	hostgroup *temp_hostgroup;
	hostsmember *temp_hostsmember;
	int odd = 0;
	char *bg_class = "";

	/* see if user is authorized to view hostgroup information... */
	if(is_authorized_for_configuration_information(&current_authdata) == FALSE) {
		unauthorized_message();
		return;
		}

	printf("<P><DIV ALIGN=CENTER CLASS='dataTitle'>%s%s%s</DIV></P>\n",_("Host Group"),
	       (*to_expand == '\0' ? "s" : " "), (*to_expand == '\0' ? "" : html_encode(to_expand, FALSE)));

	printf("<P>\n");
	printf("<DIV ALIGN=CENTER>\n");

	printf("<TABLE BORDER=0 CLASS='data'>\n");
	printf("<TR>\n");
	printf("<TH CLASS='data'>%s</TH>",_("Group Name"));
	printf("<TH CLASS='data'>%s</TH>",_("Description"));
	printf("<TH CLASS='data'>%s</TH>",_("Host Members"));
	printf("<TH CLASS='data'>%s</TH>",_("Notes"));
	printf("<TH CLASS='data'>%s</TH>",_("Notes URL"));
	printf("<TH CLASS='data'>%s</TH>",_("Action URL"));
	printf("</TR>\n");

	/* check all the hostgroups... */
	for(temp_hostgroup = hostgroup_list; temp_hostgroup != NULL; temp_hostgroup = temp_hostgroup->next) if(((*to_expand) == '\0') || (!strcmp(to_expand, temp_hostgroup->group_name))) {

			if(odd) {
				odd = 0;
				bg_class = "dataOdd";
				}
			else {
				odd = 1;
				bg_class = "dataEven";
				}

			printf("<TR CLASS='%s'>\n", bg_class);

			printf("<TD CLASS='%s'>%s</TD>", bg_class, html_encode(temp_hostgroup->group_name, FALSE));

			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, html_encode(temp_hostgroup->alias, FALSE));

			printf("<TD CLASS='%s'>", bg_class);

			/* find all the hosts that are members of this hostgroup... */
			for(temp_hostsmember = temp_hostgroup->members; temp_hostsmember != NULL; temp_hostsmember = temp_hostsmember->next) {

				if(temp_hostsmember != temp_hostgroup->members)
					printf(", ");
				printf("<A HREF='%s?type=hosts&expand=%s'>%s</A>\n", CONFIG_CGI, url_encode(temp_hostsmember->host_name), html_encode(temp_hostsmember->host_name, FALSE));
				}
			printf("</TD>\n");

			printf("<TD CLASS='%s'>%s</TD>", bg_class, (temp_hostgroup->notes == NULL) ? "&nbsp;" : html_encode(temp_hostgroup->notes, FALSE));

			printf("<TD CLASS='%s'>%s</TD>", bg_class, (temp_hostgroup->notes_url == NULL) ? "&nbsp;" : html_encode(temp_hostgroup->notes_url, FALSE));

			printf("<TD CLASS='%s'>%s</TD>", bg_class, (temp_hostgroup->action_url == NULL) ? "&nbsp;" : html_encode(temp_hostgroup->action_url, FALSE));

			printf("</TR>\n");
			}

	printf("</TABLE>\n");
	printf("</DIV>\n");
	printf("</P>\n");

	return;
	}



void display_servicegroups(void) {
	servicegroup *temp_servicegroup;
	servicesmember *temp_servicesmember;
	int odd = 0;
	char *bg_class = "";

	/* see if user is authorized to view servicegroup information... */
	if(is_authorized_for_configuration_information(&current_authdata) == FALSE) {
		unauthorized_message();
		return;
		}

	printf("<P><DIV ALIGN=CENTER CLASS='dataTitle'>%s%s%s</DIV></P>\n",_("Service Group"),
	       (*to_expand == '\0' ? "s" : " "), (*to_expand == '\0' ? "" : html_encode(to_expand, FALSE)));

	printf("<P>\n");
	printf("<DIV ALIGN=CENTER>\n");

	printf("<TABLE BORDER=0 CLASS='data'>\n");
	printf("<TR>\n");
	printf("<TH CLASS='data'>%s</TH>",_("Group Name"));
	printf("<TH CLASS='data'>%s</TH>",_("Description"));
	printf("<TH CLASS='data'>%s</TH>",_("Service Members"));
	printf("<TH CLASS='data'>%s</TH>",_("Notes"));
	printf("<TH CLASS='data'>%s</TH>",_("Notes URL"));
	printf("<TH CLASS='data'>%s</TH>",_("Action URL"));
	printf("</TR>\n");

	/* check all the servicegroups... */
	for(temp_servicegroup = servicegroup_list; temp_servicegroup != NULL; temp_servicegroup = temp_servicegroup->next) if(((*to_expand) == '\0') || (!strcmp(to_expand, temp_servicegroup->group_name))) {

			if(odd) {
				odd = 0;
				bg_class = "dataOdd";
				}
			else {
				odd = 1;
				bg_class = "dataEven";
				}

			printf("<TR CLASS='%s'>\n", bg_class);

			printf("<TD CLASS='%s'>%s</TD>", bg_class, html_encode(temp_servicegroup->group_name, FALSE));

			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, html_encode(temp_servicegroup->alias, FALSE));

			printf("<TD CLASS='%s'>", bg_class);

			/* find all the services that are members of this servicegroup... */
			for(temp_servicesmember = temp_servicegroup->members; temp_servicesmember != NULL; temp_servicesmember = temp_servicesmember->next) {

				printf("%s<A HREF='%s?type=hosts&expand=%s'>%s</A> / ", (temp_servicesmember == temp_servicegroup->members) ? "" : ", ", CONFIG_CGI, url_encode(temp_servicesmember->host_name), html_encode(temp_servicesmember->host_name, FALSE));

				printf("<A HREF='%s?type=services&expand=%s#%s;", CONFIG_CGI, url_encode(temp_servicesmember->host_name), url_encode(temp_servicesmember->host_name));
				printf("%s'>%s</A>\n", url_encode(temp_servicesmember->service_description), html_encode(temp_servicesmember->service_description, FALSE));
				}

			printf("</TD>\n");

			printf("<TD CLASS='%s'>%s</TD>", bg_class, (temp_servicegroup->notes == NULL) ? "&nbsp;" : html_encode(temp_servicegroup->notes, FALSE));

			printf("<TD CLASS='%s'>%s</TD>", bg_class, (temp_servicegroup->notes_url == NULL) ? "&nbsp;" : html_encode(temp_servicegroup->notes_url, FALSE));

			printf("<TD CLASS='%s'>%s</TD>", bg_class, (temp_servicegroup->action_url == NULL) ? "&nbsp;" : html_encode(temp_servicegroup->action_url, FALSE));

			printf("</TR>\n");
			}

	printf("</TABLE>\n");
	printf("</DIV>\n");
	printf("</P>\n");

	return;
	}



void display_contacts(void) {
	contact *temp_contact;
	commandsmember *temp_commandsmember;
	int odd = 0;
	int options;
	int found;
	char *bg_class = "";

	/* see if user is authorized to view contact information... */
	if(is_authorized_for_configuration_information(&current_authdata) == FALSE) {
		unauthorized_message();
		return;
		}

	printf("<P><DIV ALIGN=CENTER CLASS='dataTitle'>%s%s%s</DIV></P>\n",_("Contact"),
	       (*to_expand == '\0' ? "s" : " "), (*to_expand == '\0' ? "" : html_encode(to_expand, FALSE)));

	printf("<P>\n");
	printf("<DIV ALIGN=CENTER>\n");

	printf("<TABLE CLASS='data'>\n");

	printf("<TR>\n");
	printf("<TH CLASS='data'>%s</TH>",_("Contact Name"));
	printf("<TH CLASS='data'>%s</TH>",_("Alias"));
	printf("<TH CLASS='data'>%s</TH>",_("Email Address"));
	printf("<TH CLASS='data'>%s</TH>",_("Pager Address/Number"));
	printf("<TH CLASS='data'>%s</TH>",_("Service Notification Options"));
	printf("<TH CLASS='data'>%s</TH>",_("Host Notification Options"));
	printf("<TH CLASS='data'>%s</TH>",_("Service Notification Period"));
	printf("<TH CLASS='data'>%s</TH>",_("Host Notification Period"));
	printf("<TH CLASS='data'>%s</TH>",_("Service Notification Commands"));
	printf("<TH CLASS='data'>%s</TH>",_("Host Notification Commands"));
	printf("<TH CLASS='data'>%s</TH>",_("Retention Options"));
	printf("</TR>\n");

	/* check all contacts... */
	for(temp_contact = contact_list; temp_contact != NULL; temp_contact = temp_contact->next) if(((*to_expand) == '\0') || (!strcmp(to_expand, temp_contact->name))) {

			if(odd) {
				odd = 0;
				bg_class = "dataOdd";
				}
			else {
				odd = 1;
				bg_class = "dataEven";
				}

			printf("<TR CLASS='%s'>\n", bg_class);

			printf("<TD CLASS='%s'><A NAME='%s'>%s</a></TD>\n", bg_class, url_encode(temp_contact->name), html_encode(temp_contact->name, FALSE));
			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, html_encode(temp_contact->alias, FALSE));
			printf("<TD CLASS='%s'><A HREF='mailto:%s'>%s</A></TD>\n", bg_class, (temp_contact->email == NULL) ? "&nbsp;" : url_encode(temp_contact->email), (temp_contact->email == NULL) ? "&nbsp;" : html_encode(temp_contact->email, FALSE));
			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, (temp_contact->pager == NULL) ? "&nbsp;" : html_encode(temp_contact->pager, FALSE));

			printf("<TD CLASS='%s'>", bg_class);
			options = 0;
			if(temp_contact->notify_on_service_unknown == TRUE) {
				options = 1;
				printf("%s",_("Unknown"));
				}
			if(temp_contact->notify_on_service_warning == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Warning"));
				options = 1;
				}
			if(temp_contact->notify_on_service_critical == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Critical"));
				options = 1;
				}
			if(temp_contact->notify_on_service_recovery == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Recovery"));
				options = 1;
				}
			if(temp_contact->notify_on_service_flapping == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Flapping"));
				options = 1;
				}
			if(temp_contact->notify_on_service_downtime == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Downtime"));
				options = 1;
				}
			if(!options)
				printf("%s",_("None"));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			options = 0;
			if(temp_contact->notify_on_host_down == TRUE) {
				options = 1;
				printf("%s",_("Down"));
				}
			if(temp_contact->notify_on_host_unreachable == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Unreachable"));
				options = 1;
				}
			if(temp_contact->notify_on_host_recovery == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Recovery"));
				options = 1;
				}
			if(temp_contact->notify_on_host_flapping == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Flapping"));
				options = 1;
				}
			if(temp_contact->notify_on_host_downtime == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Downtime"));
				options = 1;
				}
			if(!options)
				printf("%s",_("Downtime"));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>\n", bg_class);
			if(temp_contact->service_notification_period == NULL)
				printf("&nbsp;");
			else
				printf("<A HREF='%s?type=timeperiods&expand=%s'>%s</A>", CONFIG_CGI, url_encode(temp_contact->service_notification_period), html_encode(temp_contact->service_notification_period, FALSE));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>\n", bg_class);
			if(temp_contact->host_notification_period == NULL)
				printf("&nbsp;");
			else
				printf("<A HREF='%s?type=timeperiods&expand=%s'>%s</A>", CONFIG_CGI, url_encode(temp_contact->host_notification_period), html_encode(temp_contact->host_notification_period, FALSE));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			found = FALSE;
			for(temp_commandsmember = temp_contact->service_notification_commands; temp_commandsmember != NULL; temp_commandsmember = temp_commandsmember->next) {

				if(temp_commandsmember != temp_contact->service_notification_commands)
					printf(", ");

				/* printf("<A HREF='%s?type=commands&expand=%s'>%s</A>",CONFIG_CGI,url_encode(strtok(temp_commandsmember->command,"!")),html_encode(temp_commandsmember->command,FALSE)); */
				printf("<A HREF='%s?type=command&expand=%s'>%s</A>", CONFIG_CGI, url_encode(temp_commandsmember->command), html_encode(temp_commandsmember->command, FALSE));

				found = TRUE;
				}
			if(found == FALSE)
				printf("%s",_("None"));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			found = FALSE;
			for(temp_commandsmember = temp_contact->host_notification_commands; temp_commandsmember != NULL; temp_commandsmember = temp_commandsmember->next) {

				if(temp_commandsmember != temp_contact->host_notification_commands)
					printf(", ");

				/* printf("<A HREF='%s?type=commands&expand=%s'>%s</A>",CONFIG_CGI,url_encode(strtok(temp_commandsmember->command,"!")),html_encode(temp_commandsmember->command,FALSE)); */
				printf("<A HREF='%s?type=command&expand=%s'>%s</A>", CONFIG_CGI, url_encode(temp_commandsmember->command), html_encode(temp_commandsmember->command, FALSE));

				found = TRUE;
				}
			if(found == FALSE)
				printf("%s",_("None"));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			options = 0;
			if(temp_contact->retain_status_information == TRUE) {
				options = 1;
				printf("%s",_("None"));
				}
			if(temp_contact->retain_nonstatus_information == TRUE) {
				printf("%s%s", (options == 1) ? ", " : "",_("Non-Status Information"));
				options = 1;
				}
			if(options == 0)
				printf("%s",_("None"));
			printf("</TD>\n");

			printf("</TR>\n");
			}

	printf("</TABLE>\n");
	printf("</DIV>\n");
	printf("</P>\n");

	return;
	}



void display_contactgroups(void) {
	contactgroup *temp_contactgroup;
	contactsmember *temp_contactsmember;
	int odd = 0;
	char *bg_class = "";

	/* see if user is authorized to view contactgroup information... */
	if(is_authorized_for_configuration_information(&current_authdata) == FALSE) {
		unauthorized_message();
		return;
		}


	printf("<P><DIV ALIGN=CENTER CLASS='dataTitle'>%s%s</DIV></P>\n",_("Contact Group"),
	       (*to_expand == '\0' ? "s" : " "), (*to_expand == '\0' ? "" : html_encode(to_expand, FALSE)));

	printf("<P>\n");
	printf("<DIV ALIGN=CENTER>\n");

	printf("<TABLE BORDER=0 CELLSPACING=3 CELLPADDING=0>\n");

	printf("<TR>\n");
	printf("<TH CLASS='data'>%s</TH>\n",_("Group Name"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Description"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Contact Members"));
	printf("</TR>\n");

	/* check all the contact groups... */
	for(temp_contactgroup = contactgroup_list; temp_contactgroup != NULL; temp_contactgroup = temp_contactgroup->next) if(((*to_expand) == '\0') || (!strcmp(to_expand, temp_contactgroup->group_name))) {

			if(odd) {
				odd = 0;
				bg_class = "dataOdd";
				}
			else {
				odd = 1;
				bg_class = "dataEven";
				}

			printf("<TR CLASS='%s'>\n", bg_class);

			printf("<TD CLASS='%s'><A NAME='%s'></A>%s</TD>\n", bg_class, url_encode(temp_contactgroup->group_name), html_encode(temp_contactgroup->group_name, FALSE));
			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, html_encode(temp_contactgroup->alias, FALSE));

			/* find all the contact who are members of this contact group... */
			printf("<TD CLASS='%s'>", bg_class);
			for(temp_contactsmember = temp_contactgroup->members; temp_contactsmember != NULL; temp_contactsmember = temp_contactsmember->next) {

				if(temp_contactsmember != temp_contactgroup->members)
					printf(", ");

				printf("<A HREF='%s?type=contacts&expand=%s'>%s</A>\n", CONFIG_CGI, url_encode(temp_contactsmember->contact_name), html_encode(temp_contactsmember->contact_name, FALSE));
				}
			printf("</TD>\n");

			printf("</TR>\n");
			}

	printf("</TABLE>\n");
	printf("</DIV>\n");
	printf("</P>\n");

	return;
	}



void display_services(void) {
	service *temp_service = NULL;
	contactsmember *temp_contactsmember = NULL;
	contactgroupsmember *temp_contactgroupsmember = NULL;
	char *processed_string = NULL;
	char command_line[MAX_INPUT_BUFFER];
	char *command_name = "";
	int options;
	int odd = 0;
	char time_string[16];
	char *bg_class;
	int contact = 0;


	/* see if user is authorized to view service information... */
	if(is_authorized_for_configuration_information(&current_authdata) == FALSE) {
		unauthorized_message();
		return;
		}

	printf("<P><DIV ALIGN=CENTER CLASS='dataTitle'>%s%s%s</DIV></P>\n",_("Service"),
	       (*to_expand == '\0' ? "s" : "s Named or on Host "), (*to_expand == '\0' ? "" : html_encode(to_expand, FALSE)));

	printf("<P>\n");
	printf("<DIV ALIGN=CENTER>\n");

	printf("<TABLE BORDER=0 CLASS='data'>\n");
	printf("<TR>\n");
	printf("<TH CLASS='data' COLSPAN=2>%s</TH>",_("Service"));
	printf("</TR>\n");
	printf("<TR>\n");
	printf("<TH CLASS='data'>%s</TH>\n",_("Host"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Description"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Max. Check Attempts"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Normal Check Interval"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Retry Check Interal"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Check Command"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Check Period"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Parallelize"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Volatile"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Obsess Over"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Enable Active Checks"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Enable Passive Checks"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Check Freshness"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Freshness Threshold"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Default Contacts/Groups"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Enable Notifications"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Notification Interval"));
	printf("<TH CLASS='data'>%s</TH>\n",_("First Notification Delay"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Notification Options"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Notification Period"));
	printf("<TH CLASS='data'>%s</TH>",_("Event Handler"));
	printf("<TH CLASS='data'>%s</TH>",_("Enable Event Handler"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Stalking Options"));
	printf("<TH CLASS='data'>%s</TH>",_("Enable Flap Detection"));
	printf("<TH CLASS='data'>%s</TH>",_("Low Flap Threshold"));
	printf("<TH CLASS='data'>%s</TH>",_("High Flap Threshold"));
	printf("<TH CLASS='data'>%s</TH>",_("Flap Detection Options"));
	printf("<TH CLASS='data'>%s</TH>",_("Process Performance Data"));
	printf("<TH CLASS='data'>%s</TH>",_("Enable Failure Prediction"));
	printf("<TH CLASS='data'>%s</TH>",_("Failure Prediction Options"));
	printf("<TH CLASS='data'>%s</TH>",_("Notes"));
	printf("<TH CLASS='data'>%s</TH>",_("Notes URL"));
	printf("<TH CLASS='data'>%s</TH>",_("Action URL"));
	printf("<TH CLASS='data'>%s</TH>",_("Logo Image"));
	printf("<TH CLASS='data'>%s</TH>",_("Image Alt"));
	printf("<TH CLASS='data'>%s</TH>",_("Retention Options"));
	printf("</TR>\n");

	/* check all the services... */
	for(temp_service = service_list; temp_service != NULL; temp_service = temp_service->next)
		if(((*to_expand) == '\0') || (!strcmp(to_expand, temp_service->host_name)) || (!strcmp(to_expand, temp_service->description))) {

			/* grab macros */
			grab_service_macros_r(mac, temp_service);

			if(odd) {
				odd = 0;
				bg_class = "dataOdd";
				}
			else {
				odd = 1;
				bg_class = "dataEven";
				}

			printf("<TR CLASS='%s'>\n", bg_class);

			printf("<TD CLASS='%s'><A NAME='%s;", bg_class, url_encode(temp_service->host_name));
			printf("%s'></A>", url_encode(temp_service->description));
			printf("<A HREF='%s?type=hosts&expand=%s'>%s</A></TD>\n", CONFIG_CGI, url_encode(temp_service->host_name), html_encode(temp_service->host_name, FALSE));

			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, html_encode(temp_service->description, FALSE));

			printf("<TD CLASS='%s'>%d</TD>\n", bg_class, temp_service->max_attempts);

			get_interval_time_string(temp_service->check_interval, time_string, sizeof(time_string));
			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, time_string);
			get_interval_time_string(temp_service->retry_interval, time_string, sizeof(time_string));
			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, time_string);

			strncpy(command_line, temp_service->service_check_command, sizeof(command_line));
			command_line[sizeof(command_line) - 1] = '\x0';
			command_name = strtok(strdup(command_line), "!");

			/* printf("<TD CLASS='%s'><A HREF='%s?type=commands&expand=%s'>%s</A></TD>\n",bg_class,CONFIG_CGI,url_encode(command_name),html_encode(command_line,FALSE)); */
			printf("<TD CLASS='%s'><A HREF='%s?type=command&expand=%s'>%s</A></TD>\n", bg_class, CONFIG_CGI, url_encode(command_line), html_encode(command_line, FALSE));
			free(command_name);
			printf("<TD CLASS='%s'>", bg_class);
			if(temp_service->check_period == NULL)
				printf("&nbsp;");
			else
				printf("<A HREF='%s?type=timeperiods&expand=%s'>%s</A>", CONFIG_CGI, url_encode(temp_service->check_period), html_encode(temp_service->check_period, FALSE));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, (temp_service->parallelize == TRUE) ? "Yes" : "No");

			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, (temp_service->is_volatile == TRUE) ? "Yes" : "No");

			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, (temp_service->obsess_over_service == TRUE) ? "Yes" : "No");

			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, (temp_service->checks_enabled == TRUE) ? "Yes" : "No");

			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, (temp_service->accept_passive_service_checks == TRUE) ? "Yes" : "No");

			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, (temp_service->check_freshness == TRUE) ? "Yes" : "No");

			printf("<TD CLASS='%s'>", bg_class);
			if(temp_service->freshness_threshold == 0)
				printf("%s\n",_("Auto-determined value"));
			else
				printf("%d %s\n", temp_service->freshness_threshold,_("seconds"));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			contact = 0;
			for(temp_contactsmember = temp_service->contacts; temp_contactsmember != NULL; temp_contactsmember = temp_contactsmember->next) {
				contact++;
				if(contact > 1)
					printf(", ");
				printf("<A HREF='%s?type=contacts&expand=%s'>%s</A>", CONFIG_CGI, url_encode(temp_contactsmember->contact_name), html_encode(temp_contactsmember->contact_name, FALSE));
				}
			for(temp_contactgroupsmember = temp_service->contact_groups; temp_contactgroupsmember != NULL; temp_contactgroupsmember = temp_contactgroupsmember->next) {
				contact++;
				if(contact > 1)
					printf(", ");
				printf("<A HREF='%s?type=contactgroups&expand=%s'>%s</A>\n", CONFIG_CGI, url_encode(temp_contactgroupsmember->group_name), html_encode(temp_contactgroupsmember->group_name, FALSE));
				}
			if(contact == 0)
				printf("&nbsp;");
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			printf("%s\n", (temp_service->notifications_enabled == TRUE) ? "Yes" : "No");
			printf("</TD>\n");

			get_interval_time_string(temp_service->notification_interval, time_string, sizeof(time_string));
			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, (temp_service->notification_interval == 0) ? "<i>No Re-notification</i>" : html_encode(time_string, FALSE));

			get_interval_time_string(temp_service->first_notification_delay, time_string, sizeof(time_string));
			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, time_string);

			printf("<TD CLASS='%s'>", bg_class);
			options = 0;
			if(temp_service->notify_on_unknown == TRUE) {
				options = 1;
				printf("%s",_("Unknown"));
				}
			if(temp_service->notify_on_warning == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Unknown"));
				options = 1;
				}
			if(temp_service->notify_on_critical == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Critical"));
				options = 1;
				}
			if(temp_service->notify_on_recovery == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Recovery"));
				options = 1;
				}
			if(temp_service->notify_on_flapping == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Flapping"));
				options = 1;
				}
			if(temp_service->notify_on_downtime == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Downtime"));
				options = 1;
				}
			if(!options)
				printf("%s",_("None"));
			printf("</TD>\n");
			printf("<TD CLASS='%s'>", bg_class);
			if(temp_service->notification_period == NULL)
				printf("&nbsp;");
			else
				printf("<A HREF='%s?type=timeperiods&expand=%s'>%s</A>", CONFIG_CGI, url_encode(temp_service->notification_period), html_encode(temp_service->notification_period, FALSE));
			printf("</TD>\n");
			printf("<TD CLASS='%s'>", bg_class);
			if(temp_service->event_handler == NULL)
				printf("&nbsp;");
			else
				/* printf("<A HREF='%s?type=commands&expand=%s'>%s</A>",CONFIG_CGI,url_encode(strtok(temp_service->event_handler,"!")),html_encode(temp_service->event_handler,FALSE)); */
				printf("<A HREF='%s?type=command&expand=%s'>%s</A>", CONFIG_CGI, url_encode(temp_service->event_handler), html_encode(temp_service->event_handler, FALSE));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			printf("%s\n", (temp_service->event_handler_enabled == TRUE) ? "Yes" : "No");
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			options = 0;
			if(temp_service->stalk_on_ok == TRUE) {
				options = 1;
				printf("%s",_("Ok"));
				}
			if(temp_service->stalk_on_warning == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Warning"));
				options = 1;
				}
			if(temp_service->stalk_on_unknown == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Unknown"));
				options = 1;
				}
			if(temp_service->stalk_on_critical == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Critical"));
				options = 1;
				}
			if(options == 0)
				printf("%s",_("None"));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			printf("%s\n", (temp_service->flap_detection_enabled == TRUE) ? "Yes" : "No");
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			if(temp_service->low_flap_threshold == 0.0)
				printf("%s\n",_("Program-wide value"));
			else
				printf("%3.1f%%\n", temp_service->low_flap_threshold);
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			if(temp_service->high_flap_threshold == 0.0)
				printf("%s\n",_("Program-wide value"));
			else
				printf("%3.1f%%\n", temp_service->high_flap_threshold);
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			options = 0;
			if(temp_service->flap_detection_on_ok == TRUE) {
				options = 1;
				printf("%s",_("Ok"));
				}
			if(temp_service->flap_detection_on_warning == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Warning"));
				options = 1;
				}
			if(temp_service->flap_detection_on_unknown == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Unknown"));
				options = 1;
				}
			if(temp_service->flap_detection_on_critical == TRUE) {
				printf("%s%s", (options) ? ", " : "",_("Critical"));
				options = 1;
				}
			if(options == 0)
				printf("%s",_("None"));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			printf("%s\n", (temp_service->process_performance_data == TRUE) ? "Yes" : "No");
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			printf("%s\n", (temp_service->failure_prediction_enabled == TRUE) ? "Yes" : "No");
			printf("</TD>\n");

			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, (temp_service->failure_prediction_options == NULL) ? "&nbsp;" : html_encode(temp_service->failure_prediction_options, FALSE));

			printf("<TD CLASS='%s'>%s</TD>", bg_class, (temp_service->notes == NULL) ? "&nbsp;" : html_encode(temp_service->notes, FALSE));

			printf("<TD CLASS='%s'>%s</TD>", bg_class, (temp_service->notes_url == NULL) ? "&nbsp;" : html_encode(temp_service->notes_url, FALSE));

			printf("<TD CLASS='%s'>%s</TD>", bg_class, (temp_service->action_url == NULL) ? "&nbsp;" : html_encode(temp_service->action_url, FALSE));

			if(temp_service->icon_image == NULL)
				printf("<TD CLASS='%s'>&nbsp;</TD>", bg_class);
			else {
				process_macros_r(mac, temp_service->icon_image, &processed_string, 0);
				printf("<TD CLASS='%s' valign='center'><img src='%s%s' border='0' width='20' height='20'> %s</TD>", bg_class, url_logo_images_path, processed_string, html_encode(temp_service->icon_image, FALSE));
				free(processed_string);
				}

			printf("<TD CLASS='%s'>%s</TD>", bg_class, (temp_service->icon_image_alt == NULL) ? "&nbsp;" : html_encode(temp_service->icon_image_alt, FALSE));

			printf("<TD CLASS='%s'>", bg_class);
			options = 0;
			if(temp_service->retain_status_information == TRUE) {
				options = 1;
				printf("%s",_("Status Information"));
				}
			if(temp_service->retain_nonstatus_information == TRUE) {
				printf("%s%s", (options == 1) ? ", " : "",_("Non-Status Information"));
				options = 1;
				}
			if(options == 0)
				printf("None");
			printf("</TD>\n");

			printf("</TR>\n");
			}

	printf("</TABLE>\n");
	printf("</DIV>\n");
	printf("</P>\n");

	return;
	}



void display_timeperiods(void) {
	timerange *temp_timerange = NULL;
	daterange *temp_daterange = NULL;
	timeperiod *temp_timeperiod = NULL;
	timeperiodexclusion *temp_timeperiodexclusion = NULL;
	char *months[12] = {"january", "february", "march", "april", "may", "june", "july", "august", "september", "october", "november", "december"};
	char *days[7] = {"sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday"};
	int odd = 0;
	int day = 0;
	int x = 0;
	char *bg_class = "";
	char timestring[10];
	int hours = 0;
	int minutes = 0;
	int seconds = 0;
	int line = 0;
	int item = 0;

	/* see if user is authorized to view time period information... */
	if(is_authorized_for_configuration_information(&current_authdata) == FALSE) {
		unauthorized_message();
		return;
		}

	printf("<P><DIV ALIGN=CENTER CLASS='dataTitle'>%s%s%s</DIV></P>\n",_("Time Period"),
	       (*to_expand == '\0' ? "s" : " "), (*to_expand == '\0' ? "" : html_encode(to_expand, FALSE)));

	printf("<P>\n");
	printf("<DIV ALIGN=CENTER>\n");

	printf("<TABLE BORDER=0 CLASS='data'>\n");
	printf("<TR>\n");
	printf("<TH CLASS='data'>%s</TH>\n",_("Name"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Alias/Description"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Exclusions"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Days/Dates"));
	printf("<TH CLASS='data'>%s</TH>\n",_("Times"));
	printf("</TR>\n");

	/* check all the time periods... */
	for(temp_timeperiod = timeperiod_list; temp_timeperiod != NULL; temp_timeperiod = temp_timeperiod->next) if(((*to_expand) == '\0') || (!strcmp(to_expand, temp_timeperiod->name))) {

			if(odd) {
				odd = 0;
				bg_class = "dataOdd";
				}
			else {
				odd = 1;
				bg_class = "dataEven";
				}

			printf("<TR CLASS='%s'>\n", bg_class);

			printf("<TD CLASS='%s'><A NAME='%s'>%s</A></TD>\n", bg_class, url_encode(temp_timeperiod->name), html_encode(temp_timeperiod->name, FALSE));
			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, html_encode(temp_timeperiod->alias, FALSE));

			printf("<TD CLASS='%s'>", bg_class);
			item = 0;
			for(temp_timeperiodexclusion = temp_timeperiod->exclusions; temp_timeperiodexclusion != NULL; temp_timeperiodexclusion = temp_timeperiodexclusion->next) {
				item++;
				printf("%s<A HREF='#%s'>%s</A>", (item == 1) ? "" : ",&nbsp;", url_encode(temp_timeperiodexclusion->timeperiod_name), html_encode(temp_timeperiodexclusion->timeperiod_name, FALSE));
				}
			printf("</TD>");

			printf("<TD CLASS='%s'>", bg_class);

			line = 0;
			for(x = 0; x < DATERANGE_TYPES; x++) {
				for(temp_daterange = temp_timeperiod->exceptions[x]; temp_daterange != NULL; temp_daterange = temp_daterange->next) {

					line++;

					if(line > 1)
						printf("<TR><TD COLSPAN='3'></TD><TD CLASS='%s'>\n", bg_class);

					switch(temp_daterange->type) {
						case DATERANGE_CALENDAR_DATE:
							printf("%d-%02d-%02d", temp_daterange->syear, temp_daterange->smon + 1, temp_daterange->smday);
							if((temp_daterange->smday != temp_daterange->emday) || (temp_daterange->smon != temp_daterange->emon) || (temp_daterange->syear != temp_daterange->eyear))
								printf(" - %d-%02d-%02d", temp_daterange->eyear, temp_daterange->emon + 1, temp_daterange->emday);
							if(temp_daterange->skip_interval > 1)
								printf(" / %d", temp_daterange->skip_interval);
							break;
						case DATERANGE_MONTH_DATE:
							printf("%s %d", months[temp_daterange->smon], temp_daterange->smday);
							if((temp_daterange->smon != temp_daterange->emon) || (temp_daterange->smday != temp_daterange->emday)) {
								printf(" - %s %d", months[temp_daterange->emon], temp_daterange->emday);
								if(temp_daterange->skip_interval > 1)
									printf(" / %d", temp_daterange->skip_interval);
								}
							break;
						case DATERANGE_MONTH_DAY:
							printf("day %d", temp_daterange->smday);
							if(temp_daterange->smday != temp_daterange->emday) {
								printf(" - %d", temp_daterange->emday);
								if(temp_daterange->skip_interval > 1)
									printf(" / %d", temp_daterange->skip_interval);
								}
							break;
						case DATERANGE_MONTH_WEEK_DAY:
							printf("%s %d %s", days[temp_daterange->swday], temp_daterange->swday_offset, months[temp_daterange->smon]);
							if((temp_daterange->smon != temp_daterange->emon) || (temp_daterange->swday != temp_daterange->ewday) || (temp_daterange->swday_offset != temp_daterange->ewday_offset)) {
								printf(" - %s %d %s", days[temp_daterange->ewday], temp_daterange->ewday_offset, months[temp_daterange->emon]);
								if(temp_daterange->skip_interval > 1)
									printf(" / %d", temp_daterange->skip_interval);
								}
							break;
						case DATERANGE_WEEK_DAY:
							printf("%s %d", days[temp_daterange->swday], temp_daterange->swday_offset);
							if((temp_daterange->swday != temp_daterange->ewday) || (temp_daterange->swday_offset != temp_daterange->ewday_offset)) {
								printf(" - %s %d", days[temp_daterange->ewday], temp_daterange->ewday_offset);
								if(temp_daterange->skip_interval > 1)
									printf(" / %d", temp_daterange->skip_interval);
								}
							break;
						default:
							break;
						}

					printf("</TD><TD CLASS='%s'>\n", bg_class);

					for(temp_timerange = temp_daterange->times; temp_timerange != NULL; temp_timerange = temp_timerange->next) {

						if(temp_timerange != temp_daterange->times)
							printf(", ");

						hours = temp_timerange->range_start / 3600;
						minutes = (temp_timerange->range_start - (hours * 3600)) / 60;
						seconds = temp_timerange->range_start - (hours * 3600) - (minutes * 60);
						snprintf(timestring, sizeof(timestring) - 1, "%02d:%02d:%02d", hours, minutes, seconds);
						timestring[sizeof(timestring) - 1] = '\x0';
						printf("%s - ", timestring);

						hours = temp_timerange->range_end / 3600;
						minutes = (temp_timerange->range_end - (hours * 3600)) / 60;
						seconds = temp_timerange->range_end - (hours * 3600) - (minutes * 60);
						snprintf(timestring, sizeof(timestring) - 1, "%02d:%02d:%02d", hours, minutes, seconds);
						timestring[sizeof(timestring) - 1] = '\x0';
						printf("%s", timestring);
						}

					printf("</TD>\n");
					printf("</TR>\n");
					}
				}
			for(day = 0; day < 7; day++) {

				if(temp_timeperiod->days[day] == NULL)
					continue;

				line++;

				if(line > 1)
					printf("<TR><TD COLSPAN='3'></TD><TD CLASS='%s'>\n", bg_class);

				printf("%s", days[day]);

				printf("</TD><TD CLASS='%s'>\n", bg_class);

				for(temp_timerange = temp_timeperiod->days[day]; temp_timerange != NULL; temp_timerange = temp_timerange->next) {

					if(temp_timerange != temp_timeperiod->days[day])
						printf(", ");

					hours = temp_timerange->range_start / 3600;
					minutes = (temp_timerange->range_start - (hours * 3600)) / 60;
					seconds = temp_timerange->range_start - (hours * 3600) - (minutes * 60);
					snprintf(timestring, sizeof(timestring) - 1, "%02d:%02d:%02d", hours, minutes, seconds);
					timestring[sizeof(timestring) - 1] = '\x0';
					printf("%s - ", timestring);

					hours = temp_timerange->range_end / 3600;
					minutes = (temp_timerange->range_end - (hours * 3600)) / 60;
					seconds = temp_timerange->range_end - (hours * 3600) - (minutes * 60);
					snprintf(timestring, sizeof(timestring) - 1, "%02d:%02d:%02d", hours, minutes, seconds);
					timestring[sizeof(timestring) - 1] = '\x0';
					printf("%s", timestring);
					}

				printf("&nbsp;</TD>\n");
				printf("</TR>\n");
				}

			if(line == 0) {
				printf("</TD>\n");
				printf("</TR>\n");
				}
			}

	printf("</TABLE>\n");
	printf("</DIV>\n");
	printf("</P>\n");

	return;
	}



void display_commands(void) {
	command *temp_command;
	int odd = 0;
	char *bg_class = "";

	/* see if user is authorized to view command information... */
	if(is_authorized_for_configuration_information(&current_authdata) == FALSE) {
		unauthorized_message();
		return;
		}

	printf("<P><DIV ALIGN=CENTER CLASS='dataTitle'>%s%s%s</DIV></P>\n",_("Command"),
	       (*to_expand == '\0' ? "s" : " "), (*to_expand == '\0' ? "" : html_encode(to_expand, FALSE)));

	printf("<P><DIV ALIGN=CENTER>\n");
	printf("<TABLE BORDER=0 CLASS='data'>\n");
	printf("<TR><TH CLASS='data'>%s</TH><TH CLASS='data'>%s</TH></TR>\n",_("Command Name"),_("Command Line"));

	/* check all commands */
	for(temp_command = command_list; temp_command != NULL; temp_command = temp_command->next) if(((*to_expand) == '\0') || (!strcmp(to_expand, temp_command->name))) {

			if(odd) {
				odd = 0;
				bg_class = "dataEven";
				}
			else {
				odd = 1;
				bg_class = "dataOdd";
				}

			printf("<TR CLASS='%s'>\n", bg_class);

			printf("<TD CLASS='%s'><A NAME='%s'></A>%s</TD>\n", bg_class, url_encode(temp_command->name), html_encode(temp_command->name, FALSE));
			printf("<TD CLASS='%s'>%s</TD>\n", bg_class, html_encode(temp_command->command_line, FALSE));

			printf("</TR>\n");
			}

	printf("</TABLE>\n");
	printf("</DIV></P>\n");

	return;
	}



void display_servicedependencies(void) {
	servicedependency *temp_sd;
	int odd = 0;
	int options;
	char *bg_class = "";

	/* see if user is authorized to view hostgroup information... */
	if(is_authorized_for_configuration_information(&current_authdata) == FALSE) {
		unauthorized_message();
		return;
		}

	printf("<P><DIV ALIGN=CENTER CLASS='dataTitle'>%s%s%s</DIV></P>\n",_("Service Dependencie"),
	       (*to_expand == '\0' ? "s" : "s Involving Host "), (*to_expand == '\0' ? "" : html_encode(to_expand, FALSE)));

	printf("<P>\n");
	printf("<DIV ALIGN=CENTER>\n");

	printf("<TABLE BORDER=0 CLASS='data'>\n");
	printf("<TR>\n");
	printf("<TH CLASS='data' COLSPAN=2>%s</TH>",_("Dependent Service"));
	printf("<TH CLASS='data' COLSPAN=2>%s</TH>",_("Master Service"));
	printf("</TR>\n");
	printf("<TR>\n");
	printf("<TH CLASS='data'>%s</TH>",_("Host"));
	printf("<TH CLASS='data'>%s</TH>",_("Service"));
	printf("<TH CLASS='data'>%s</TH>",_("Host"));
	printf("<TH CLASS='data'>%s</TH>",_("Service"));
	printf("<TH CLASS='data'>%s</TH>",_("Dependency Type"));
	printf("<TH CLASS='data'>%s</TH>",_("Dependency Period"));
	printf("<TH CLASS='data'>%s</TH>",_("Dependency Failure Options"));
	printf("</TR>\n");

	/* check all the service dependencies... */
	for(temp_sd = servicedependency_list; temp_sd != NULL; temp_sd = temp_sd->next)
		if(((*to_expand) == '\0') || (!strcmp(to_expand, temp_sd->dependent_host_name)) || (!strcmp(to_expand, temp_sd->host_name))) {

			if(odd) {
				odd = 0;
				bg_class = "dataOdd";
				}
			else {
				odd = 1;
				bg_class = "dataEven";
				}

			printf("<TR CLASS='%s'>\n", bg_class);

			printf("<TD CLASS='%s'><A HREF='%s?type=hosts&expand=%s'>%s</A></TD>", bg_class, CONFIG_CGI, url_encode(temp_sd->dependent_host_name), html_encode(temp_sd->dependent_host_name, FALSE));

			printf("<TD CLASS='%s'><A HREF='%s?type=services&expand=%s#%s;", bg_class, CONFIG_CGI, url_encode(temp_sd->dependent_host_name), url_encode(temp_sd->dependent_host_name));
			printf("%s'>%s</A></TD>\n", url_encode(temp_sd->dependent_service_description), html_encode(temp_sd->dependent_service_description, FALSE));

			printf("<TD CLASS='%s'><A HREF='%s?type=hosts&expand=%s'>%s</A></TD>", bg_class, CONFIG_CGI, url_encode(temp_sd->host_name), html_encode(temp_sd->host_name, FALSE));

			printf("<TD CLASS='%s'><A HREF='%s?type=services&expand=%s#%s;", bg_class, CONFIG_CGI, url_encode(temp_sd->host_name), url_encode(temp_sd->host_name));
			printf("%s'>%s</A></TD>\n", url_encode(temp_sd->service_description), html_encode(temp_sd->service_description, FALSE));

			printf("<TD CLASS='%s'>%s</TD>", bg_class, (temp_sd->dependency_type == NOTIFICATION_DEPENDENCY) ? "Notification" : "Check Execution");

			printf("<TD CLASS='%s'>", bg_class);
			if(temp_sd->dependency_period == NULL)
				printf("&nbsp;");
			else
				printf("<A HREF='%s?type=timeperiods&expand=%s'>%s</A>", CONFIG_CGI, url_encode(temp_sd->dependency_period), html_encode(temp_sd->dependency_period, FALSE));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			options = FALSE;
			if(temp_sd->fail_on_ok == TRUE) {
				printf("%s",_("Ok"));
				options = TRUE;
				}
			if(temp_sd->fail_on_warning == TRUE) {
				printf("%s%s", (options == TRUE) ? ", " : "",_("Warning"));
				options = TRUE;
				}
			if(temp_sd->fail_on_unknown == TRUE) {
				printf("%s%s", (options == TRUE) ? ", " : "",_("Unknown"));
				options = TRUE;
				}
			if(temp_sd->fail_on_critical == TRUE) {
				printf("%s%s", (options == TRUE) ? ", " : "",_("Critical"));
				options = TRUE;
				}
			if(temp_sd->fail_on_pending == TRUE) {
				printf("%s%s", (options == TRUE) ? ", " : "",_("Pending"));
				options = TRUE;
				}
			printf("</TD>\n");

			printf("</TR>\n");
			}

	printf("</TABLE>\n");
	printf("</DIV>\n");
	printf("</P>\n");

	return;
	}



void display_serviceescalations(void) {
	serviceescalation *temp_se = NULL;
	contactsmember *temp_contactsmember = NULL;
	contactgroupsmember *temp_contactgroupsmember = NULL;
	char time_string[16] = "";
	int options = FALSE;
	int odd = 0;
	char *bg_class = "";
	int contact = 0;

	/* see if user is authorized to view hostgroup information... */
	if(is_authorized_for_configuration_information(&current_authdata) == FALSE) {
		unauthorized_message();
		return;
		}

	printf("<P><DIV ALIGN=CENTER CLASS='dataTitle'>%s%s%s</DIV></P>\n",_("Service Escalation"),
	       (*to_expand == '\0' ? "s" : "s on Host "), (*to_expand == '\0' ? "" : html_encode(to_expand, FALSE)));

	printf("<P>\n");
	printf("<DIV ALIGN=CENTER>\n");

	printf("<TABLE BORDER=0 CLASS='data'>\n");
	printf("<TR>\n");
	printf("<TH CLASS='data' COLSPAN=2>%s</TH>",_("Service"));
	printf("</TR>\n");
	printf("<TR>\n");
	printf("<TH CLASS='data'>%s</TH>",_("Host"));
	printf("<TH CLASS='data'>%s</TH>",_("Description"));
	printf("<TH CLASS='data'>%s</TH>",_("Contacts/Groups"));
	printf("<TH CLASS='data'>%s</TH>",_("First Notification"));
	printf("<TH CLASS='data'>%s</TH>",_("Last Notification"));
	printf("<TH CLASS='data'>%s</TH>",_("Notification Interval"));
	printf("<TH CLASS='data'>%s</TH>",_("Escalation Period"));
	printf("<TH CLASS='data'>%s</TH>",_("Escalation Options"));
	printf("</TR>\n");

	/* check all the service escalations... */
	for(temp_se = serviceescalation_list; temp_se != NULL; temp_se = temp_se->next) if(((*to_expand) == '\0') || (!strcmp(to_expand, temp_se->host_name))) {

			if(odd) {
				odd = 0;
				bg_class = "dataOdd";
				}
			else {
				odd = 1;
				bg_class = "dataEven";
				}

			printf("<TR CLASS='%s'>\n", bg_class);

			printf("<TD CLASS='%s'><A HREF='%s?type=hosts&expand=%s'>%s</A></TD>", bg_class, CONFIG_CGI, url_encode(temp_se->host_name), html_encode(temp_se->host_name, FALSE));

			printf("<TD CLASS='%s'><A HREF='%s?type=services&expand=%s#%s;", bg_class, CONFIG_CGI, url_encode(temp_se->host_name), url_encode(temp_se->host_name));
			printf("%s'>%s</A></TD>\n", url_encode(temp_se->description), html_encode(temp_se->description, FALSE));

			printf("<TD CLASS='%s'>", bg_class);
			contact = 0;
			for(temp_contactsmember = temp_se->contacts; temp_contactsmember != NULL; temp_contactsmember = temp_contactsmember->next) {
				contact++;
				if(contact > 1)
					printf(", ");
				printf("<A HREF='%s?type=contacts&expand=%s'>%s</A>\n", CONFIG_CGI, url_encode(temp_contactsmember->contact_name), html_encode(temp_contactsmember->contact_name, FALSE));
				}
			for(temp_contactgroupsmember = temp_se->contact_groups; temp_contactgroupsmember != NULL; temp_contactgroupsmember = temp_contactgroupsmember->next) {
				contact++;
				if(contact > 1)
					printf(", ");
				printf("<A HREF='%s?type=contactgroups&expand=%s'>%s</A>\n", CONFIG_CGI, url_encode(temp_contactgroupsmember->group_name), html_encode(temp_contactgroupsmember->group_name, FALSE));
				}
			if(contact == 0)
				printf("&nbsp;");
			printf("</TD>\n");

			printf("<TD CLASS='%s'>%d</TD>", bg_class, temp_se->first_notification);

			printf("<TD CLASS='%s'>", bg_class);
			if(temp_se->last_notification == 0)
				printf("%s",_("Infinity"));
			else
				printf("%d", temp_se->last_notification);
			printf("</TD>\n");

			get_interval_time_string(temp_se->notification_interval, time_string, sizeof(time_string));
			printf("<TD CLASS='%s'>", bg_class);
			if(temp_se->notification_interval == 0.0)
				printf("%s",_("Notify Only Once (No Re-notification)"));
			else
				printf("%s", time_string);
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			if(temp_se->escalation_period == NULL)
				printf("&nbsp;");
			else
				printf("<A HREF='%s?type=timeperiods&expand=%s'>%s</A>", CONFIG_CGI, url_encode(temp_se->escalation_period), html_encode(temp_se->escalation_period, FALSE));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			options = FALSE;
			if(temp_se->escalate_on_warning == TRUE) {
				printf("%s%s", (options == TRUE) ? ", " : "",_("Warning"));
				options = TRUE;
				}
			if(temp_se->escalate_on_unknown == TRUE) {
				printf("%s%s", (options == TRUE) ? ", " : "",_("Unknown"));
				options = TRUE;
				}
			if(temp_se->escalate_on_critical == TRUE) {
				printf("%s%s", (options == TRUE) ? ", " : "",_("Critical"));
				options = TRUE;
				}
			if(temp_se->escalate_on_recovery == TRUE) {
				printf("%s%s", (options == TRUE) ? ", " : "",_("Recovery"));
				options = TRUE;
				}
			if(options == FALSE)
				printf("%s",_("None"));
			printf("</TD>\n");

			printf("</TR>\n");
			}

	printf("</TABLE>\n");
	printf("</DIV>\n");
	printf("</P>\n");

	return;
	}



void display_hostdependencies(void) {
	hostdependency *temp_hd;
	int odd = 0;
	int options;
	char *bg_class = "";

	/* see if user is authorized to view hostdependency information... */
	if(is_authorized_for_configuration_information(&current_authdata) == FALSE) {
		unauthorized_message();
		return;
		}

	printf("<P><DIV ALIGN=CENTER CLASS='dataTitle'>%s%s%s</DIV></P>\n",_("Host Dependencie"),
	       (*to_expand == '\0' ? "s" : "s Involving Host "), (*to_expand == '\0' ? "" : html_encode(to_expand, FALSE)));

	printf("<P>\n");
	printf("<DIV ALIGN=CENTER>\n");

	printf("<TABLE BORDER=0 CLASS='data'>\n");
	printf("<TR>\n");
	printf("<TH CLASS='data'>%s</TH>",_("Dependent Host"));
	printf("<TH CLASS='data'>%s</TH>",_("Master Host"));
	printf("<TH CLASS='data'>%s</TH>",_("Dependency Type"));
	printf("<TH CLASS='data'>%s</TH>",_("Dependency Period"));
	printf("<TH CLASS='data'>%s</TH>",_("Dependency Failure Options"));
	printf("</TR>\n");

	/* check all the host dependencies... */
	for(temp_hd = hostdependency_list; temp_hd != NULL; temp_hd = temp_hd->next)
		if(((*to_expand) == '\0') || (!strcmp(to_expand, temp_hd->dependent_host_name)) || (!strcmp(to_expand, temp_hd->host_name))) {

			if(odd) {
				odd = 0;
				bg_class = "dataOdd";
				}
			else {
				odd = 1;
				bg_class = "dataEven";
				}

			printf("<TR CLASS='%s'>\n", bg_class);

			printf("<TD CLASS='%s'><A HREF='%s?type=hosts&expand=%s'>%s</A></TD>", bg_class, CONFIG_CGI, url_encode(temp_hd->dependent_host_name), html_encode(temp_hd->dependent_host_name, FALSE));

			printf("<TD CLASS='%s'><A HREF='%s?type=hosts&expand=%s'>%s</A></TD>", bg_class, CONFIG_CGI, url_encode(temp_hd->host_name), html_encode(temp_hd->host_name, FALSE));

			printf("<TD CLASS='%s'>%s</TD>", bg_class, (temp_hd->dependency_type == NOTIFICATION_DEPENDENCY) ? "Notification" : "Check Execution");

			printf("<TD CLASS='%s'>", bg_class);
			if(temp_hd->dependency_period == NULL)
				printf("&nbsp;");
			else
				printf("<A HREF='%s?type=timeperiods&expand=%s'>%s</A>", CONFIG_CGI, url_encode(temp_hd->dependency_period), html_encode(temp_hd->dependency_period, FALSE));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			options = FALSE;
			if(temp_hd->fail_on_up == TRUE) {
				printf("%s",_("Up"));
				options = TRUE;
				}
			if(temp_hd->fail_on_down == TRUE) {
				printf("%s%s", (options == TRUE) ? ", " : "",_("Down"));
				options = TRUE;
				}
			if(temp_hd->fail_on_unreachable == TRUE) {
				printf("%s%s", (options == TRUE) ? ", " : "",_("Unreachable"));
				options = TRUE;
				}
			if(temp_hd->fail_on_pending == TRUE) {
				printf("%s%s", (options == TRUE) ? ", " : "",_("Pending"));
				options = TRUE;
				}
			printf("</TD>\n");

			printf("</TR>\n");
			}

	printf("</TABLE>\n");
	printf("</DIV>\n");
	printf("</P>\n");

	return;
	}



void display_hostescalations(void) {
	hostescalation *temp_he = NULL;
	contactsmember *temp_contactsmember = NULL;
	contactgroupsmember *temp_contactgroupsmember = NULL;
	char time_string[16] = "";
	int options = FALSE;
	int odd = 0;
	char *bg_class = "";
	int contact = 0;

	/* see if user is authorized to view hostgroup information... */
	if(is_authorized_for_configuration_information(&current_authdata) == FALSE) {
		unauthorized_message();
		return;
		}

	printf("<P><DIV ALIGN=CENTER CLASS='dataTitle'>%s%s%s</DIV></P>\n",_("Host Escalation"),
	       (*to_expand == '\0' ? "s" : "s for Host "), (*to_expand == '\0' ? "" : html_encode(to_expand, FALSE)));

	printf("<P>\n");
	printf("<DIV ALIGN=CENTER>\n");

	printf("<TABLE BORDER=0 CLASS='data'>\n");
	printf("<TR>\n");
	printf("<TH CLASS='data'>%s</TH>",_("Host"));
	printf("<TH CLASS='data'>%s</TH>",_("Contacts/Groups"));
	printf("<TH CLASS='data'>%s</TH>",_("First Notification"));
	printf("<TH CLASS='data'>%s</TH>",_("Last Notification"));
	printf("<TH CLASS='data'>%s</TH>",_("Notification Interval"));
	printf("<TH CLASS='data'>%s</TH>",_("Escalation Period"));
	printf("<TH CLASS='data'>%s</TH>",_("Escalation Options"));
	printf("</TR>\n");

	/* check all the host escalations... */
	for(temp_he = hostescalation_list; temp_he != NULL; temp_he = temp_he->next) if(((*to_expand) == '\0') || (!strcmp(to_expand, temp_he->host_name))) {

			if(odd) {
				odd = 0;
				bg_class = "dataOdd";
				}
			else {
				odd = 1;
				bg_class = "dataEven";
				}

			printf("<TR CLASS='%s'>\n", bg_class);

			printf("<TD CLASS='%s'><A HREF='%s?type=hosts&expand=%s'>%s</A></TD>", bg_class, CONFIG_CGI, url_encode(temp_he->host_name), html_encode(temp_he->host_name, FALSE));

			printf("<TD CLASS='%s'>", bg_class);
			contact = 0;
			for(temp_contactsmember = temp_he->contacts; temp_contactsmember != NULL; temp_contactsmember = temp_contactsmember->next) {
				contact++;
				if(contact > 1)
					printf(", ");
				printf("<A HREF='%s?type=contacts&expand=%s'>%s</A>\n", CONFIG_CGI, url_encode(temp_contactsmember->contact_name), html_encode(temp_contactsmember->contact_name, FALSE));
				}
			for(temp_contactgroupsmember = temp_he->contact_groups; temp_contactgroupsmember != NULL; temp_contactgroupsmember = temp_contactgroupsmember->next) {
				contact++;
				if(contact > 1)
					printf(", ");
				printf("<A HREF='%s?type=contactgroups&expand=%s'>%s</A>\n", CONFIG_CGI, url_encode(temp_contactgroupsmember->group_name), html_encode(temp_contactgroupsmember->group_name, FALSE));
				}
			if(contact == 0)
				printf("&nbsp;");
			printf("</TD>\n");

			printf("<TD CLASS='%s'>%d</TD>", bg_class, temp_he->first_notification);

			printf("<TD CLASS='%s'>", bg_class);
			if(temp_he->last_notification == 0)
				printf("%s",_("Infinity"));
			else
				printf("%d", temp_he->last_notification);
			printf("</TD>\n");

			get_interval_time_string(temp_he->notification_interval, time_string, sizeof(time_string));
			printf("<TD CLASS='%s'>", bg_class);
			if(temp_he->notification_interval == 0.0)
				printf("%s",_("Notify Only Once (No Re-notification)"));
			else
				printf("%s", time_string);
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			if(temp_he->escalation_period == NULL)
				printf("&nbsp;");
			else
				printf("<A HREF='%s?type=timeperiods&expand=%s'>%s</A>", CONFIG_CGI, url_encode(temp_he->escalation_period), html_encode(temp_he->escalation_period, FALSE));
			printf("</TD>\n");

			printf("<TD CLASS='%s'>", bg_class);
			options = FALSE;
			if(temp_he->escalate_on_down == TRUE) {
				printf("%s%s", (options == TRUE) ? ", " : "",_("Down"));
				options = TRUE;
				}
			if(temp_he->escalate_on_unreachable == TRUE) {
				printf("%s%s", (options == TRUE) ? ", " : "",_("Unreachable"));
				options = TRUE;
				}
			if(temp_he->escalate_on_recovery == TRUE) {
				printf("%s%s", (options == TRUE) ? ", " : "",_("Recovery"));
				options = TRUE;
				}
			if(options == FALSE)
				printf("%s",_("None"));
			printf("</TD>\n");

			printf("</TR>\n");
			}

	printf("</TABLE>\n");
	printf("</DIV>\n");
	printf("</P>\n");

	return;
	}



void unauthorized_message(void) {

	printf("<P><DIV CLASS='errorMessage'>%s</DIV></P>\n",_("It appears as though you do not have permission to view the configuration information you requested..."));
	printf("<P><DIV CLASS='errorDescription'>%s</DIV></P>\n",_("If you believe this is an error, check the HTTP server authentication requirements for accessing this CGI<br>and check the authorization options in your CGI configuration file."));

	return;
	}




char *hash_color(int i) {
	char c;

	/* This is actually optimized for MAX_COMMAND_ARGUMENTS==32 ... */

	if((i % 32) < 16) {
		if((i % 32) < 8) c = '7';
		else c = '4';
		}
	else {
		if((i % 32) < 24) c = '6';
		else c = '5';
		}

	/* Computation for standard case */
	hashed_color[0] = '#';
	hashed_color[1] = hashed_color[2] = ((i % 2) ? c : '0');
	hashed_color[3] = hashed_color[4] = (((i / 2) % 2) ? c : '0');
	hashed_color[5] = hashed_color[6] = (((i / 4) % 2) ? c : '0');
	hashed_color[7] = '\0';

	/* Override shades of grey */
	if((i % 8) == 7) hashed_color[1] = hashed_color[3] = '0';
	if((i % 8) == 0) hashed_color[2] = hashed_color[3] = hashed_color[4] = hashed_color[6] = c;

	return(hashed_color);
	}

void display_command_expansion(void) {
	command *temp_command;
	int odd = 0;
	char *bg_class = "";
	int i, j;
	char *c, *cc;
	char commandline[MAX_COMMAND_BUFFER];
	char *command_args[MAX_COMMAND_ARGUMENTS];
	int arg_count[MAX_COMMAND_ARGUMENTS],
	    lead_space[MAX_COMMAND_ARGUMENTS],
	    trail_space[MAX_COMMAND_ARGUMENTS];

	/* see if user is authorized to view command information... */
	if(is_authorized_for_configuration_information(&current_authdata) == FALSE) {
		unauthorized_message();
		return;
		}

	printf("<P><DIV ALIGN=CENTER CLASS='dataTitle'>Command Expansion</DIV></P>\n");

	/* Parse to_expand into parts */
	for(i = 0; i < MAX_COMMAND_ARGUMENTS; i++) command_args[i] = NULL;
	for(i = 0, command_args[0] = cc = c = strdup(to_expand); c && ((*c) != '\0') && (i < MAX_COMMAND_ARGUMENTS); c++, cc++) {
		if((*c) == '\\') c++;
		else if((*c) == '!') {
			(*cc) = '\0';
			cc = c++;
			command_args[++i] = (c--);
			}
		(*cc) = (*c);
		}
	if((*c) == '\0')(*cc) = '\0';
	/* Precompute indexes of dangling whitespace */
	for(i = 0; i < MAX_COMMAND_ARGUMENTS; i++) {
		for(cc = command_args[i], lead_space[i] = 0; cc && isspace(*cc); cc++, lead_space[i]++) ;
		trail_space[i] = 0;
		for(; cc && ((*cc) != '\0'); cc++) if(isspace(*cc)) trail_space[i]++;
			else trail_space[i] = 0;
		}

	printf("<P><DIV ALIGN=CENTER>\n");
	printf("<TABLE BORDER=0 CLASS='data'>\n");
	printf("<TR><TH CLASS='data'>Command Name</TH><TH CLASS='data'>Command Line</TH></TR>\n");

	if((*to_expand) != '\0') {
		arg_count[0] = 0;

		printf("<TR CLASS='dataEven'><TD CLASS='dataEven'>To expand:</TD><TD CLASS='dataEven'>%s", escape_string(command_args[0]));
		for(i = 1; (i < MAX_COMMAND_ARGUMENTS) && command_args[i]; i++)
			printf("!<FONT\n   COLOR='%s'>%s</FONT>", hash_color(i), escape_string(command_args[i]));
		printf("\n</TD></TR>\n");

		/* check all commands */
		for(temp_command = command_list; temp_command != NULL; temp_command = temp_command->next) {

			if(!strcmp(temp_command->name, command_args[0])) {

				arg_count[0]++;

				if(odd) {
					odd = 0;
					bg_class = "dataEven";
					}
				else {
					odd = 1;
					bg_class = "dataOdd";
					}

				printf("<TR CLASS='%s'>\n", bg_class);

				printf("<TD CLASS='%s'><A NAME='%s'></A>%s</TD>\n", bg_class, url_encode(temp_command->name), html_encode(temp_command->name, FALSE));
				printf("<TD CLASS='%s'>%s</TD>\n", bg_class, html_encode(temp_command->command_line, FALSE));

				printf("</TR>\n<TR CLASS='%s'>\n", bg_class);

				for(i = 1; i < MAX_COMMAND_ARGUMENTS; i++) arg_count[i] = 0;

				printf("<TD CLASS='%s' ALIGN='right'>-&gt;</TD>\n", bg_class);
				printf("<TD CLASS='%s'>", bg_class);
				strncpy(commandline, temp_command->command_line, MAX_COMMAND_BUFFER);
				commandline[MAX_COMMAND_BUFFER - 1] = '\0';
				for(c = commandline; c && (cc = strstr(c, "$"));) {
					(*(cc++)) = '\0';
					printf("%s", html_encode(c, FALSE));
					if((*cc) == '$') {
						/* Escaped '$' */
						printf("<FONT COLOR='#444444'>$</FONT>");
						c = (++cc);
						}
					else if(strncmp("ARG", cc, 3)) {
						/* Non-$ARGn$ macro */
						c = strstr(cc, "$");
						if(c)(*(c++)) = '\0';
						printf("<FONT COLOR='#777777'>$%s%s</FONT>", html_encode(cc, FALSE), (c ? "$" : ""));
						if(!c) printf("<FONT COLOR='#FF0000'> (not properly terminated)</FONT>");
						}
					else {
						/* $ARGn$ macro */
						for(c = (cc += 3); isdigit(*c); c++) ;
						if(((*c) == '\0') || ((*c) == '$')) {
							/* Index is numeric */
							i = atoi(cc);
							if((i > 0) && (i <= MAX_COMMAND_ARGUMENTS)) {
								arg_count[i]++;
								if(command_args[i]) {
									if(*(command_args[i]) != '\0') printf("<FONT COLOR='%s'><B>%s%s%s</B></FONT>",
										                                      hash_color(i), ((lead_space[i] > 0) || (trail_space[i] > 0) ? "<U>&zwj;" : ""),
										                                      html_encode(command_args[i], FALSE), ((lead_space[i] > 0) || (trail_space[i] > 0) ? "&zwj;</U>" : ""));
									else printf("<FONT COLOR='#0000FF'>(empty)</FONT>");
									}
								else printf("<FONT COLOR='#0000FF'>(undefined)</FONT>");
								}
							else printf("<FONT COLOR='#FF0000'>(not a valid $ARGn$ index: %u)</FONT>", i);
							if((*c) != '\0') c++;
							else printf("<FONT COLOR='#FF0000'> (not properly terminated)</FONT>");
							}
						else {
							/* Syntax err in index */
							c = strstr(cc, "$");
							printf("<FONT COLOR='#FF0000'>(not an $ARGn$ index: &quot;%s&quot;)</FONT>", html_encode(strtok(cc, "$"), FALSE));
							if(c) c++;
							}
						}
					}
				if(c) printf("%s", html_encode(c, FALSE));

				printf("</TD></TR>\n");

				for(i = 1; (i < MAX_COMMAND_ARGUMENTS) && (command_args[i]); i++) {
					if(arg_count[i] == 0) {
						printf("<TR CLASS='%s'><TD CLASS='%s' ALIGN='right'><FONT COLOR='#FF0000'>unused:</FONT></TD>\n", bg_class, bg_class);
						printf("<TD CLASS='%s'>$ARG%u$=<FONT COLOR='%s'>%s%s%s</FONT></TD></TR>\n", bg_class, i, hash_color(i),
						       ((lead_space[i] > 0) || (trail_space[i] > 0) ? "<U>&zwj;" : ""), html_encode(command_args[i], FALSE),
						       ((lead_space[i] > 0) || (trail_space[i] > 0) ? "&zwj;</U>" : ""));
						}
					else if(arg_count[i] > 1) {
						printf("<TR CLASS='%s'><TD CLASS='%s' ALIGN='right'>used %u x:</TD>\n", bg_class, bg_class, i);
						printf("<TD CLASS='%s'>$ARG%u$=<FONT COLOR='%s'>%s%s%s</FONT></TD></TR>\n", bg_class, i, hash_color(i),
						       ((lead_space[i] > 0) || (trail_space[i] > 0) ? "<U>&zwj;" : ""), html_encode(command_args[i], FALSE),
						       ((lead_space[i] > 0) || (trail_space[i] > 0) ? "&zwj;</U>" : ""));
						}
					if((lead_space[i] > 0) || (trail_space[i] > 0)) {
						printf("<TR CLASS='%s'><TD CLASS='%s' ALIGN='right'><FONT COLOR='#0000FF'>dangling whitespace:</FONT></TD>\n", bg_class, bg_class);
						printf("<TD CLASS='%s'>$ARG%u$=<FONT COLOR='#0000FF'>", bg_class, i);
						for(c = command_args[i], j = 0; c && isspace(*c); c++, j++)
							/* TODO: As long as the hyperlinks change all whitespace into actual spaces,
							   we'll output "[WS]" (whitespace) instead of "[SP]"(ace). */
							/* if ((*c)==' ')		printf("[SP]"); */
							if((*c) == ' ')		printf("[WS]");
							else if((*c) == '\f')	printf("[FF]");
							else if((*c) == '\n')	printf("[LF]");
							else if((*c) == '\r')	printf("[CR]");
							else if((*c) == '\t')	printf("[HT]");
							else if((*c) == '\v')	printf("[VT]");
							else			printf("[0x%x]", *c);
						printf("</FONT><FONT COLOR='%s'>", hash_color(i));
						for(; c && ((*c) != '\0') && (j < strlen(command_args[i]) - trail_space[i]); c++, j++) putchar(*c);
						printf("</FONT><FONT COLOR='#0000FF'>");
						for(; c && ((*c) != '\0'); c++)
							/* TODO: As long as the hyperlinks change all whitespace into actual spaces,
							   we'll output "[WS]" (whitespace) instead of "[SP]"(ace). */
							/* if ((*c)==' ')		printf("[SP]"); */
							if((*c) == ' ')		printf("[WS]");
							else if((*c) == '\f')	printf("[FF]");
							else if((*c) == '\n')	printf("[LF]");
							else if((*c) == '\r')	printf("[CR]");
							else if((*c) == '\t')	printf("[HT]");
							else if((*c) == '\v')	printf("[VT]");
							else			printf("[0x%x]", *c);
						printf("</FONT></TD></TR>\n");
						}
					}
				}

			}

		if(!arg_count[0]) {
			printf("<TR CLASS='dataOdd'><TD CLASS='dataOdd' ALIGN='right'><FONT\n");
			printf("COLOR='#FF0000'>Error:</FONT></TD><TD CLASS='dataOdd'><FONT COLOR='#FF0000'>No\n");
			printf("command &quot;%s&quot; found</FONT></TD></TR>\n", html_encode(command_args[0], FALSE));
			}
		}

	printf("<TR CLASS='dataEven'><TD><BR/></TD><TD CLASS='dataEven'>%s</TD></TR>\n",_("Enter the command_check definition from a host or service definition and press Go to see the expansion of the command"));
	printf("<TR CLASS='dataEven'><TD CLASS='dataEven'>To expand:</TD><TD CLASS='dataEven'><FORM\n");
	printf("METHOD='GET' ACTION='%s'><INPUT TYPE='HIDDEN' NAME='type' VALUE='command'><INPUT\n", CONFIG_CGI);
	printf("TYPE='text' NAME='expand' SIZE='100%%' VALUE='%s'>\n", html_encode(to_expand, FALSE));
	printf("<INPUT TYPE='SUBMIT' VALUE='Go'></FORM></TD></TR>\n");

	printf("</TABLE>\n");
	printf("</DIV></P>\n");

	return;
	}



void display_options(void) {

	printf("<br><br>\n");

	printf("<div align=center class='reportSelectTitle'>%s</div>\n",_("Select Type of Config Data You Wish To View"));

	printf("<br><br>\n");

	printf("<form method=\"get\" action=\"%s\">\n", CONFIG_CGI);

	printf("<div align=center>\n");
	printf("<table border=0>\n");

	printf("<tr><td align=left class='reportSelectSubTitle'>%s:</td></tr>\n",_("Object Type"));
	printf("<tr><td align=left class='reportSelectItem'>");
	printf("<select name='type'>\n");
	printf("<option value='hosts' %s>%s\n", (display_type == DISPLAY_HOSTS) ? "SELECTED" : "",_("Hosts"));
	printf("<option value='hostdependencies' %s>%s\n", (display_type == DISPLAY_HOSTDEPENDENCIES) ? "SELECTED" : "",_("Host Dependencies"));
	printf("<option value='hostescalations' %s>%s\n", (display_type == DISPLAY_HOSTESCALATIONS) ? "SELECTED" : "",_("Host Escalations"));
	printf("<option value='hostgroups' %s>%s\n", (display_type == DISPLAY_HOSTGROUPS) ? "SELECTED" : "",_("Host Groups"));
	printf("<option value='services' %s>%s\n", (display_type == DISPLAY_SERVICES) ? "SELECTED" : "",_("Services"));
	printf("<option value='servicegroups' %s>%s\n", (display_type == DISPLAY_SERVICEGROUPS) ? "SELECTED" : "",_("Service Groups"));
	printf("<option value='servicedependencies' %s>%s\n", (display_type == DISPLAY_SERVICEDEPENDENCIES) ? "SELECTED" : "",_("Service Dependencies"));
	printf("<option value='serviceescalations' %s>%s\n", (display_type == DISPLAY_SERVICEESCALATIONS) ? "SELECTED" : "",_("Service Escalations"));
	printf("<option value='contacts' %s>%s\n", (display_type == DISPLAY_CONTACTS) ? "SELECTED" : "",_("Contacts"));
	printf("<option value='contactgroups' %s>%s\n", (display_type == DISPLAY_CONTACTGROUPS) ? "SELECTED" : "",_("Contact Groups"));
	printf("<option value='timeperiods' %s>%s\n", (display_type == DISPLAY_TIMEPERIODS) ? "SELECTED" : "",_("Timeperiods"));
	printf("<option value='commands' %s>%s\n", (display_type == DISPLAY_COMMANDS) ? "SELECTED" : "",_("Commands"));
	printf("<option value='command' %s>Command Expansion\n", (display_type == DISPLAY_COMMAND_EXPANSION) ? "SELECTED" : "");
	printf("</select>\n");
	printf("</td></tr>\n");

	printf("<tr><td class='reportSelectItem'><input type='submit' value='%s'></td></tr>\n",_("Continue"));
	printf("</table>\n");
	printf("</div>\n");

	printf("</form>\n");

	return;
	}
