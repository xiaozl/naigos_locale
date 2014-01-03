<?php
include_once(dirname(__FILE__).'/includes/utils.inc.php');

$link_target="main";
?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">

<html>

<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="ROBOTS" content="NOINDEX, NOFOLLOW">
<TITLE>Nagios Core</TITLE>
<link href="stylesheets/common.css" type="text/css" rel="stylesheet">
</head>

<body class='navbar'>



<div class="navbarlogo">
<a href="http://www.nagios.org" target="_blank"><img src="images/sblogo.png" height="39" width="140" border="0" alt="Nagios" /></a>
</div>

<div class="navsection">
<div class="navsectiontitle"><?php echo(__R('General')) ?></div>
<div class="navsectionlinks">
<ul class="navsectionlinks">
<li><a href="main.php" target="<?php echo $link_target;?>"><?php echo(__R('Home')) ?></a></li>
<li><a href="http://go.nagios.com/nagioscore/docs" target="_blank">Documentation</a></li>
</ul>
</div>
</div>


<div class="navsection">
<div class="navsectiontitle"><?php echo(__R('Current Status')) ?></div>
<div class="navsectionlinks">
<ul class="navsectionlinks">
<li><a href="<?php echo $cfg["cgi_base_url"];?>/tac.cgi" target="<?php echo $link_target;?>"><?php echo(__R('Tactical Overview')) ?></a></li>
<li><a href="<?php echo $cfg["cgi_base_url"];?>/statusmap.cgi?host=all" target="<?php echo $link_target;?>"><?php echo(__R('Map')) ?></a></li>
<li><a href="<?php echo $cfg["cgi_base_url"];?>/status.cgi?hostgroup=all&amp;style=hostdetail" target="<?php echo $link_target;?>"><?php echo(__R('Hosts')) ?></a></li>
<li><a href="<?php echo $cfg["cgi_base_url"];?>/status.cgi?host=all" target="<?php echo $link_target;?>"><?php echo(__R('Services')) ?></a></li>
<li>
<a href="<?php echo $cfg["cgi_base_url"];?>/status.cgi?hostgroup=all&amp;style=overview" target="<?php echo $link_target;?>"><?php echo(__R('Host Groups')) ?></a>
<ul>
<li><a href="<?php echo $cfg["cgi_base_url"];?>/status.cgi?hostgroup=all&amp;style=summary" target="<?php echo $link_target;?>"><?php echo(__R('Summary')) ?></a></li>
<li><a href="<?php echo $cfg["cgi_base_url"];?>/status.cgi?hostgroup=all&amp;style=grid" target="<?php echo $link_target;?>"><?php echo(__R('Grid')) ?></a></li>
</ul>
</li>
<li>
<a href="<?php echo $cfg["cgi_base_url"];?>/status.cgi?servicegroup=all&amp;style=overview" target="<?php echo $link_target;?>"><?php echo(__R('Service Groups')) ?></a>
<ul>
<li><a href="<?php echo $cfg["cgi_base_url"];?>/status.cgi?servicegroup=all&amp;style=summary" target="<?php echo $link_target;?>"><?php echo(__R('Summary')) ?></a></li>
<li><a href="<?php echo $cfg["cgi_base_url"];?>/status.cgi?servicegroup=all&amp;style=grid" target="<?php echo $link_target;?>"><?php echo(__R('Grid')) ?></a></li>
</ul>
</li>
<li>
<a href="<?php echo $cfg["cgi_base_url"];?>/status.cgi?host=all&amp;servicestatustypes=28" target="<?php echo $link_target;?>"><?php echo(__R('Problems')) ?></a>
<ul>
<li>
<a href="<?php echo $cfg["cgi_base_url"];?>/status.cgi?host=all&amp;servicestatustypes=28" target="<?php echo $link_target;?>"><?php echo(__R('Services')) ?></a> (<a href="<?php echo $cfg["cgi_base_url"];?>/status.cgi?host=all&amp;type=detail&amp;hoststatustypes=3&amp;serviceprops=42&amp;servicestatustypes=28" target="<?php echo $link_target;?>"><?php echo(__R('Unhandled')) ?></a>)
</li>
<li>
<a href="<?php echo $cfg["cgi_base_url"];?>/status.cgi?hostgroup=all&amp;style=hostdetail&amp;hoststatustypes=12" target="<?php echo $link_target;?>"><?php echo(__R('Hosts')) ?></a> (<a href="<?php echo $cfg["cgi_base_url"];?>/status.cgi?hostgroup=all&amp;style=hostdetail&amp;hoststatustypes=12&amp;hostprops=42" target="<?php echo $link_target;?>"><?php echo(__R('Unhandled')) ?></a>)
</li>
<li><a href="<?php echo $cfg["cgi_base_url"];?>/outages.cgi" target="<?php echo $link_target;?>"><?php echo(__R('Network Outages')) ?></a></li>
</ul>
</li>
</ul>
</div>


<div class="navbarsearch">
<form method="get" action="<?php echo $cfg["cgi_base_url"];?>/status.cgi" target="<?php echo $link_target;?>">
<fieldset>
<legend><?php echo(__R('Quick Search')) ?>:</legend>
<input type='hidden' name='navbarsearch' value='1'>
<input type='text' name='host' size='15' class="NavBarSearchItem">
</fieldset>
</form>
</div>

</div>


<div class="navsection">
<div class="navsectiontitle"><?php echo(__R('Reports')) ?></div>
<div class="navsectionlinks">
<ul class="navsectionlinks">
<li><a href="<?php echo $cfg["cgi_base_url"];?>/avail.cgi" target="<?php echo $link_target;?>"><?php echo(__R('Availability')) ?></a></li>
<li><a href="<?php echo $cfg["cgi_base_url"];?>/trends.cgi" target="<?php echo $link_target;?>"><?php echo(__R('Trends')) ?></a></li>

<li>
<a href="<?php echo $cfg["cgi_base_url"];?>/history.cgi?host=all" target="<?php echo $link_target;?>"><?php echo(__R('Alerts')) ?></a>
<ul>
<li><a href="<?php echo $cfg["cgi_base_url"];?>/history.cgi?host=all" target="<?php echo $link_target;?>"><?php echo(__R('History')) ?></a></li>
<li><a href="<?php echo $cfg["cgi_base_url"];?>/summary.cgi" target="<?php echo $link_target;?>"><?php echo(__R('Summary')) ?></a></li>
<li><a href="<?php echo $cfg["cgi_base_url"];?>/histogram.cgi" target="<?php echo $link_target;?>"><?php echo(__R('Histogram')) ?></a></li>
</ul>
</li>
<li><a href="<?php echo $cfg["cgi_base_url"];?>/notifications.cgi?contact=all" target="<?php echo $link_target;?>"><?php echo(__R('Notifications')) ?></a></li>
<li><a href="<?php echo $cfg["cgi_base_url"];?>/showlog.cgi" target="<?php echo $link_target;?>"><?php echo(__R('Event Log')) ?></a></li>
</ul>
</div>
</div>


<div class="navsection">
<div class="navsectiontitle"><?php echo(__R('System')) ?></div>
<div class="navsectionlinks">
<ul class="navsectionlinks">
<li><a href="<?php echo $cfg["cgi_base_url"];?>/extinfo.cgi?type=3" target="<?php echo $link_target;?>"><?php echo(__R('Comments')) ?></a></li>
<li><a href="<?php echo $cfg["cgi_base_url"];?>/extinfo.cgi?type=6" target="<?php echo $link_target;?>"><?php echo(__R('Downtime')) ?></a></li>
<li><a href="<?php echo $cfg["cgi_base_url"];?>/extinfo.cgi?type=0" target="<?php echo $link_target;?>"><?php echo(__R('Process Info')) ?></a></li>
<li><a href="<?php echo $cfg["cgi_base_url"];?>/extinfo.cgi?type=4" target="<?php echo $link_target;?>"><?php echo(__R('Performance Info')) ?></a></li>
<li><a href="<?php echo $cfg["cgi_base_url"];?>/extinfo.cgi?type=7" target="<?php echo $link_target;?>"><?php echo(__R('Scheduling Queue')) ?></a></li>
<li><a href="<?php echo $cfg["cgi_base_url"];?>/config.cgi" target="<?php echo $link_target;?>"><?php echo(__R('Configuration')) ?></a></li>
</ul>
</div>
</div>




</BODY>
</HTML>
