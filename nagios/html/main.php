<?php
include_once(dirname(__FILE__).'/includes/utils.inc.php');

$this_version="3.5.1";

?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">

<html>

<head>
<meta http-equiv='content-type' content='text/html;charset=UTF-8'>
<meta name="ROBOTS" content="NOINDEX, NOFOLLOW" />
<title>Nagios Core</title>
<link rel='stylesheet' type='text/css' href='stylesheets/common.css' />
<script type="text/javascript" src="js/jquery-1.7.1.min.js"></script>

<script type='text/javascript'>

	//rss fetch by ajax to reduce page load time
	$(document).ready(function() {		
		 $('#splashbox2-contents').load('rss-corefeed.php');				
		 $('#splashbox4-contents').load('rss-newsfeed.php');	
	}); 

</script>

</head>


<body id="splashpage">


<div id="mainbrandsplash">
<div id="mainlogo">
<a href="http://www.nagios.org/" target="_blank"><img src="images/logofullsize.png" border="0" alt="Nagios" title="Nagios"></a>
</div>
</div>

<div id="currentversioninfo">
<div class="product"><?php echo __R('Nagios')?><sup><span style="font-size: small;">&reg;</span></sup> <?php echo __R('Core')?><sup><span style="font-size: small;">&trade;</span></sup></div>
<div class="version"><?php echo __R('Version 3.5.1')?></div>
<div class="releasedate"><?php echo __R('August 30, 2013')?></div>
<div class="checkforupdates"><a href="http://www.nagios.org/checkforupdates/?version=3.5.1&product=nagioscore" target="_blank"><?php echo __R('Check for updates')?></a></div>
<!--<div class="whatsnew"><a href="http://go.nagios.com/nagioscore/whatsnew">Read what's new in Nagios Core 3</a></div>-->
</div>


<div id="updateversioninfo">
<?php
	$updateinfo=get_update_information();
	//print_r($updateinfo);
	//$updateinfo['update_checks_enabled']=false;
	//$updateinfo['update_available']=true;
	if($updateinfo['update_checks_enabled']==false){
?>
		<div class="updatechecksdisabled">
		<div class="warningmessage"><?php echo __R('Warning: Automatic Update Checks are Disabled!')?></div>
		<div class="submessage"> <?php echo __R('Disabling update checks presents a possible security risk.  Visit')?><a href="http://www.nagios.org/" target="_blank">nagios.org</a> <?php echo __R('to check for updates manually or enable update checks in your Nagios config file.')?></a></div>
		</div>
<?php
		}
	else if($updateinfo['update_available']==true && $this_version!=$updateinfo['update_version']){
?>
		<div class="updateavailable">
		<div class="updatemessage"><?php echo __R('A new version of Nagios Core is available!')?></div>
		<div class="submessage">Visit <a href="http://www.nagios.org/download/" target="_blank">nagios.org</a> to download Nagios <?php echo $updateinfo['update_version'];?>.</div>
		</div>
<?php
		}
?>
</div>



<div id="splashboxes">
	<div id='topsplashbox'>
		<div id="splashbox1" class="splashbox">	
			<h2><?php echo __R('Get Started')?></h2>
			<ul>
			<li><a href="http://go.nagios.com/nagioscore/startmonitoring" target="_blank"><?php echo __R('Start monitoring your infrastructure')?></a></li>
			<li><a href="http://go.nagios.com/nagioscore/changelook" target="_blank"><?php echo __R('Change the look and feel of Nagios')?></a></li>
			<li><a href="http://go.nagios.com/nagioscore/extend" target="_blank"><?php echo __R('Extend Nagios with hundreds of addons')?></a></li>
			<!--<li><a href="http://go.nagios.com/nagioscore/docs" target="_blank">Read the Nagios documentation</a></li>-->
			<li><a href="http://go.nagios.com/nagioscore/support" target="_blank"><?php echo __R('Get support')?></a></li>
			<li><a href="http://go.nagios.com/nagioscore/training" target="_blank"><?php echo __R('Get training')?></a></li>
			<li><a href="http://go.nagios.com/nagioscore/certification" target="_blank"><?php echo __R('Get certified')?></a></li>
			</ul>
		</div> <!-- end splashbox1 -->
		
		<!-- corepromo feed -->
		<div id="splashbox2" class="splashbox">
		<h2><?php echo __R("Don't Miss...")?></h2>
		<div id="splashbox2-contents"></div>
		</div>
		
	</div> <!-- end topsplashbox -->
	
	<div id="bottomsplashbox">

		<div id="splashbox3" class="splashbox">
			<h2><?php echo __R('Quick Links')?></h2>
			<ul>
				<li><a href="http://library.nagios.com" target="_blank"><?php echo __R('Nagios Library')?></a> (<?php echo __R('tutorials and docs')?>)</li>
				<li><a href="http://labs.nagios.com" target="_blank"><?php echo __R('Nagios Labs')?></a> (<?php echo __R('development blog')?>)</li>
				<li><a href="http://exchange.nagios.org" target="_blank"><?php echo __R('Nagios Exchange')?></a> (<?php echo __R('plugins and addons')?>)</li>
				<li><a href="http://support.nagios.com" target="_blank"><?php echo __R('Nagios Support')?></a> (<?php echo __R('tech support')?>)</li>
				<li><a href="http://www.nagios.com" target="_blank">Nagios.com</a> (<?php echo __R('company')?>)</li>
				<li><a href="http://www.nagios.org" target="_blank">Nagios.org</a> (<?php echo __R('project')?>)</li>

			</ul>
		</div><!-- end splashbox3 -->

		<!-- latest news feed -->
		<div id="splashbox4" class="splashbox">
		<h2><?php echo __R('Latest News')?></h2>
		<div id="splashbox4-contents"></div>
		</div>
	</div> <!-- end bottomsplashbox -->
</div><!--splashboxes-->


<div id="mainfooter">
<div id="maincopy">Copyright &copy; 2010-<?php echo date("Y");?> Nagios Core Development Team and Community Contributors. Copyright &copy; 1999-2009 Ethan Galstad. See the THANKS file for more information on contributors.</div>
<div CLASS="disclaimer">
Nagios Core is licensed under the GNU General Public License and is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.  Nagios, Nagios Core and the Nagios logo are trademarks, servicemarks, registered trademarks or registered servicemarks owned by Nagios Enterprises, LLC.  Use of the Nagios marks is governed by the <A href="http://www.nagios.com/legal/trademarks/">trademark use restrictions</a>.
</div>
<div class="logos">
<!--<a href="http://www.nagios.com/" target="_blank"><img src="images/NagiosEnterprises-whitebg-112x46.png" width="112" height="46" border="0" style="padding: 0 20px 0 0;" title="Nagios Enterprises"></a>  -->

<a href="http://www.nagios.org/" target="_blank"><img src="images/weblogo1.png" width="102" height="47" border="0" style="padding: 0 40px 0 40px;" title="Nagios.org"></a>

<a href="http://sourceforge.net/projects/nagios"><img src="images/sflogo.png" width="88" height="31" border="0" alt="SourceForge.net Logo" /></a>
</div>
</div> 


</body>
</html>

