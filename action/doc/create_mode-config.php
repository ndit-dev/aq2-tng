<?php
  //-----------------------------------------------------------------------------
  // create_mode-config.php
  //-----------------------------------------------------------------------------
  // $Id: create_mode-config.php,v 1.3 2001/06/20 07:29:10 igor_rock Exp $
  //-----------------------------------------------------------------------------

  // Do some checks for illegal game modes
  if ($data[matchmode])
    {
    $data[ctf]         = 0;
    $data[use_3teams]  = 0;
    $data[use_tourney] = 0;
    $data[teamplay]    = 1;
    }

  if ($data[ctf])
    {
    $data[use_3teams]  = 0;
    $data[use_tourney] = 0;
    $data[teamplay]    = 1;
    }
  
  if ($data[use_tourney] and $data[use_3teams])
    {
    $data[use_3teams]  = 0;
    }

  if ($data[use_3teams])
    {
    $data[teamplay]    = 1;
    }

  if ($data[teamplay])
    {
    $data[deathmatch]  = 1;
    }

  // check if the user wants to see or to save it
  if ($data[action] == "Show Config")
    {  
    header ("Content-type: text/plain");
    }
  else
    {
    header ("Content-type: application/aq2tng-config");
    }
  header ("Content-Disposition: attachment; filename=$data[configname]");
  header ("Content-Description: AQ2:TNG configfile");
 
  print "//-----------------------------------------------------------------------------\n";
  print "// AQ2:TNG Mode Config File $data[configname]\n";
  print "//\n";
  print "// created: ".date("Y-m-d, H:i:s")."\n";
  print "// with the AQ2:TNG config generator at http://www.FIXME.com\n";
  print "//-----------------------------------------------------------------------------\n\n";

  print "\n//-----------------------------------------------------------------------------\n";
  print "// General Settings\n";
  print "//-----------------------------------------------------------------------------\n";
  print "set hostname \"$data[hostname]\"\n";
  print "set password \"$data[password]\"\n";
  $dmflags = 0;
  for ($i = 0; $i < 20; $i++)
    {
    $dmflags += $data[dmflag][$i];
    }
  print "set dmflags $dmflags\n";
  print "set maplistname $data[maplistname]\n";
  print "set ininame $data[ininame]\n";
  print "set maxclients $data[maxclients]\n";
  print "set punishkills $data[punishkills]\n";
  print "set noscore $data[noscore]\n";
  print "set nohud $data[nohud]\n";
  print "set use_warnings $data[use_warnings]\n";
  print "set use_rewards $data[use_rewards]\n";

  print "\n//-----------------------------------------------------------------------------\n";
  print "// Game Mode Settings\n";
  print "//-----------------------------------------------------------------------------\n";
  print "set deathmatch $data[deathmatch]\n";
  print "set teamplay $data[teamplay]\n";
  print "set ctf $data[ctf]\n";
  print "set use_3teams $data[use_3teams]\n";
  print "set use_tourney $data[use_tourney]\n";
  print "set matchmode $data[matchmode]\n";
  
  print "\n//-----------------------------------------------------------------------------\n";
  print "// Limits\n";
  print "//-----------------------------------------------------------------------------\n";
  print "set fraglimit $data[fraglimit]\n";
  print "set timelimit $data[timelimit]\n";
  print "set roundlimit $data[roundlimit]\n";
  print "set roundtimelimit $data[roundtimelimit]\n";
  print "set limchasecam $data[limchasecam]\n";
  print "set knifelimit $data[knifelimit]\n";

  print "\n//-----------------------------------------------------------------------------\n";
  print "// Weapon and Item Options\n";
  print "//-----------------------------------------------------------------------------\n";
  print "set allweapon $data[allweapon]\n";
  print "set weapons $data[weapons]\n";
  $wp_flags = 0;
  for ($i = 0; $i < 20; $i++)
    {
    $wp_flags += $data[wp_flags][$i];
    }
  print "set wp_flags $wp_flags\n";
  print "set allitem $data[allitem]\n";
  print "set items $data[items]\n";
  $itm_flags = 0;
  for ($i = 0; $i < 20; $i++)
    {
    $itm_flags += $data[itm_flags][$i];
    }
  print "set itm_flags $itm_flags\n";
  print "set ir $data[ir]\n";
  print "set new_irvision $data[new_irvision]\n";
  print "set tgren $data[tgren]\n";
  print "set dmweapon \"$data[dmweapon]\"\n";
  print "set hc_single $data[hc_single]\n";

  print "\n//-----------------------------------------------------------------------------\n";
  print "// CTF Settings\n";
  print "//-----------------------------------------------------------------------------\n";
# Not yet implemented
#  print "set ctf_mode $data[ctf_mode]\n";
  print "set ctf_dropflag $data[ctf_dropflag]\n";
  print "set capturelimit $data[capturelimit]\n";

  print "\n//-----------------------------------------------------------------------------\n";
  print "// Video Checking\n";
  print "//-----------------------------------------------------------------------------\n";
  print "set video_check $data[video_check]\n";
  print "set video_force_restart $data[video_force_restart]\n";
  print "set video_check_lockpvs $data[video_check_lockpvs]\n";
  print "set video_checktime $data[video_checktime]\n";
  print "set video_max_3dfx $data[video_max_3dfx]\n";
  print "set video_max_3dfxam $data[video_max_3dfxam]\n";
  print "set video_max_opengl $data[video_max_opengl]\n";

  print "\n//-----------------------------------------------------------------------------\n";
  print "// Config End\n";
  print "//-----------------------------------------------------------------------------\n";

  print "map $data[startmap]\n";
?>