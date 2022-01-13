<?php

if ( isset($_GET['ledState'] ) )
{
	$ledFile = fopen("led.txt","w");
	
	if ( $_GET['ledState'] == '1' )
	{
		fwrite($ledFile,'1');
	}
	elseif ( $_GET['ledState'] == '0' )
	{
		fwrite($ledFile,'0');
	}
	
	fclose($ledFile);
}