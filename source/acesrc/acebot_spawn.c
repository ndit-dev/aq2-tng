///////////////////////////////////////////////////////////////////////
//
//  ACE - Quake II Bot Base Code
//
//  Version 1.0
//
//  Original file is Copyright(c), Steve Yeager 1998, All Rights Reserved
//
//
//	All other files are Copyright(c) Id Software, Inc.
////////////////////////////////////////////////////////////////////////
/*
 * $Header: /LTK2/src/acesrc/acebot_spawn.c 6     24/02/00 20:05 Riever $
 *
 * $Log: /LTK2/src/acesrc/acebot_spawn.c $
 * 
 * 6     24/02/00 20:05 Riever
 * Team Radio use fixed up. (Using ACE to test)
 * 
 * Manual addition: Radio 'treport' support added.
 * User: Riever       Date: 21/02/00   Time: 23:42
 * Changed initial spawn state to BS_ROAM
 * User: Riever       Date: 21/02/00   Time: 15:16
 * Bot now has the ability to roam on dry land. Basic collision functions
 * written. Active state skeletal implementation.
 * User: Riever       Date: 20/02/00   Time: 20:27
 * Added new members and definitions ready for 2nd generation of bots.
 * User: Riever       Date: 17/02/00   Time: 17:10
 * Radiogender now set for bots.
 * 
 */

///////////////////////////////////////////////////////////////////////
//
//  acebot_spawn.c - This file contains all of the 
//                   spawing support routines for the ACE bot.
//
///////////////////////////////////////////////////////////////////////

// Define this for an ACE bot
#define ACE_SPAWN
// ----

#include "../g_local.h"
#include "../m_player.h"

#include "acebot.h"
#include "botchat.h"
#include "botscan.h"
#include <time.h>

//AQ2 ADD
#define	CONFIG_FILE_VERSION 1

void	ClientBeginDeathmatch( edict_t *ent );
void	AllItems( edict_t *ent );
void	AllWeapons( edict_t *ent );
void	EquipClient( edict_t *ent );
char	*TeamName(int team);
void	LTKsetBotName( char	*bot_name );
void	LTKsetBotNameNew(void);
void	ACEAI_Cmd_Choose( edict_t *ent, char *s);

//==========================================
// Joining a team (hacked from AQ2 code)
//==========================================
/*	defined in a_team.h
#define NOTEAM          0
#define TEAM1           1
#define TEAM2           2
*/

//==============================
// Get the number of the next team a bot should join
//==============================
int GetNextTeamNumber()
{
        int i, onteam1 = 0, onteam2 = 0, onteam3 = 0;
        edict_t *e;

        // only use this function during [2]team games...
        if (!teamplay->value )
                return 0;

//		gi.bprintf(PRINT_HIGH, "Checking team balance..\n");
        for (i = 1; i <= game.maxclients; i++)
        {
                e = g_edicts + i;
                if (e->inuse)
                {
                    if (e->client->resp.team == TEAM1)
                        onteam1++;
                    else if (e->client->resp.team == TEAM2)
                        onteam2++;
                    else if (e->client->resp.team == TEAM3)
                        onteam3++;
                }
        }

	// Return the team number that needs the next bot
        if (use_3teams->value && (onteam3 < onteam1) && (onteam3 < onteam2))
                return (3);
        else if (onteam2 < onteam1)
                return (2);
        //default
        return (1);
}

//==========================
// Join a Team
//==========================
void ACESP_JoinTeam(edict_t *ent, int desired_team)
{
	JoinTeam( ent, desired_team, true );
	CheckForUnevenTeams( ent );
}

//======================================
// ACESP_LoadBotConfig()
//======================================
// Using RiEvEr's new config file
//
void ACESP_LoadBotConfig()
{
    FILE	*pIn;
	cvar_t	*game_dir = NULL, *botdir = NULL;
#ifdef _WIN32
	int		i;
#endif
	char	filename[60];
	// Scanner stuff
	int		fileVersion = 0;
	char	inString[81];
	char	tokenString[81];
	char	*sp, *tp;
	int		ttype;

	game_dir = gi.cvar ("game", "action", 0);
	botdir = gi.cvar ("botdir", "bots", 0);

if (ltk_loadbots->value){

	// Turning off stat collection since bots are enabled
	gi.cvar_forceset(stat_logs->name, "0");
	// Try to load the file for THIS level
	#ifdef	_WIN32
		i =  sprintf(filename, ".\\");
		i += sprintf(filename + i, game_dir->string);
		i += sprintf(filename + i, "\\");
		i += sprintf(filename + i, botdir->string);
		i += sprintf(filename + i, "\\");
		i += sprintf(filename + i, level.mapname);
		i += sprintf(filename + i, ".cfg");
	#else
		strcpy(filename, "./");
		strcat(filename, game_dir->string);
		strcat(filename, "/");
		strcat(filename, botdir->string);
		strcat(filename, "/");
		strcat(filename, level.mapname);
		strcat(filename, ".cfg");
	#endif

		// If there's no specific file for this level, then
		// load the file name from value ltk_botfile (default is botdata.cfg)
		if((pIn = fopen(filename, "rb" )) == NULL)
		{
	#ifdef	_WIN32
			i =  sprintf(filename, ".\\");
			i += sprintf(filename + i, game_dir->string);
			i += sprintf(filename + i, "\\");
			i += sprintf(filename + i, botdir->string);
			i += sprintf(filename + i, "\\");
			i += sprintf(filename + i, ltk_botfile->string);
			i += sprintf(filename + i, ".cfg");
	#else
			strcpy(filename, "./");
			strcat(filename, game_dir->string);
			strcat(filename, "/");
			strcat(filename, botdir->string);
			strcat(filename, "/");
			strcat(filename, ltk_botfile->string);
			strcat(filename, ".cfg");
	#endif

			// No bot file available, get out of here!
			if((pIn = fopen(filename, "rb" )) == NULL) {
				gi.dprintf("WARNING: No file containing bot data was found, no bots loaded.\n");
				gi.dprintf("ltk_botfile value is %s\n", ltk_botfile->string);
				return; // bail
			}
		}

		// Now scan each line for information
		// First line should be the file version number
		fgets( inString, 80, pIn );
		sp = inString;
		tp = tokenString;
		ttype = UNDEF;

		// Scan it for the version number
		scanner( &sp, tp, &ttype );
		if(ttype == BANG)
		{
			scanner( &sp, tp, &ttype );
			if(ttype == INTLIT)
			{
				fileVersion = atoi( tokenString );
			}
			if( fileVersion != CONFIG_FILE_VERSION )
			{
				// ERROR!
				gi.bprintf(PRINT_HIGH, "Bot Config file is out of date!\n");
				fclose(pIn);
				return;
			}
		}

		// Now process each line of the config file
		while( fgets(inString, 80, pIn) )
		{
			ACESP_SpawnBotFromConfig( inString );
		}

			

	/*	fread(&count,sizeof (int),1,pIn); 

		for(i=0;i<count;i++)
		{
			fread(userinfo,sizeof(char) * MAX_INFO_STRING,1,pIn); 
			ACESP_SpawnBot (NULL, NULL, NULL, userinfo);
		}*/
			
		fclose(pIn);
	}
}

//===========================
// ACESP_SpawnBotFromConfig
//===========================
// Input string is from the config file and should have
// all the data we need to spawn a bot
//
edict_t *ACESP_SpawnBotFromConfig( char *inString )
{
	edict_t	*bot = NULL;
	char	userinfo[MAX_INFO_STRING];
	int		count=1;
	char	name[32];
	char	modelskin[80];
	int		team=0, weaponchoice=0, equipchoice=0;
	char	gender[2] = "m";
	char	team_str[2] = "0";

	// Scanner stuff
	char	tokenString[81];
	char	*sp, *tp;
	int		ttype;

	sp = inString;
	ttype = UNDEF;

	while( ttype != EOL )
	{
		tp = tokenString;
		scanner(&sp, tp, &ttype);

		// Check for comments
		if( ttype == HASH || ttype == LEXERR)
			return NULL;

		// Check for semicolon (end of input marker)
		if( ttype == SEMIC || ttype == EOL)
			continue;

		// Keep track of which parameter we are reading
		if( ttype == COMMA )
		{
			count++;
			continue;
		}

		// NAME (parameter 1)
		if(count == 1 && ttype == STRLIT)
		{
//			strncpy( name, tokenString, 32 );
			strcpy( name, tokenString);
			continue;
		}

		// MODELSKIN (parameter 2)
		if(count == 2 && ttype == STRLIT)
		{
//			strncpy( modelskin, tokenString, 32 );
			strcpy( modelskin, tokenString);
			continue;
		}

		if(count == 3 && ttype == INTLIT )
		{
			team = atoi(tokenString);
			continue;
		}

		if(count == 4 && ttype == INTLIT )
		{
			weaponchoice = atoi(tokenString);
			continue;
		}

		if(count == 5 && ttype == INTLIT )
		{
			equipchoice = atoi(tokenString);
			continue;
		}

		if(count == 6 && ttype == SYMBOL )
		{
			gender[0] = tokenString[0];
			continue;
		}

	}// End while
	
	// To allow bots to respawn
	// initialise userinfo
	memset( userinfo, 0, sizeof(userinfo) );
	
	// add bot's name/skin/hand to userinfo
	Info_SetValueForKey( userinfo, "name", name );
	Info_SetValueForKey( userinfo, "skin", modelskin );
	Info_SetValueForKey( userinfo, "hand", "2" ); // bot is center handed for now!
	Info_SetValueForKey( userinfo, "spectator", "0" ); // NOT a spectator
	Info_SetValueForKey( userinfo, "gender", gender );
	
	// Only spawn from config if attract mode is disabled
	if (!attract_mode->value) {
		bot = ACESP_SpawnBot( team_str, name, modelskin, userinfo );
	} else {
		gi.dprintf("Warning: attract_mode is enabled, I am not spawning bots from config.\n");
	}

	// FIXME: This might have to happen earlier to take effect.
	if( bot )
	{
		bot->weaponchoice = weaponchoice;
		bot->equipchoice = equipchoice;
	}
	
	return bot;
}
//AQ2 END


///////////////////////////////////////////////////////////////////////
// Called by PutClient in Server to actually release the bot into the game
// Keep from killin' each other when all spawned at once
///////////////////////////////////////////////////////////////////////
void ACESP_HoldSpawn(edict_t *self)
{
	if (!KillBox (self))
	{	// could't spawn in?
	}

	gi.linkentity (self);

#ifdef	ACE_SPAWN
	// Use ACE bots
	self->think = ACEAI_Think;
#else
	self->think = BOTAI_Think;
#endif

	self->nextthink = level.framenum + 1;

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_LOGIN);
	gi.multicast (self->s.origin, MULTICAST_PVS);

/*	if(ctf->value)
	gi.bprintf(PRINT_MEDIUM, "%s joined the %s team.\n",
		self->client->pers.netname, CTFTeamName(self->client->resp.ctf_team));
	else*/
		gi.bprintf (PRINT_MEDIUM, "%s entered the game\n", self->client->pers.netname);

}

///////////////////////////////////////////////////////////////////////
// Modified version of id's code
///////////////////////////////////////////////////////////////////////
void ACESP_PutClientInServer( edict_t *bot, qboolean respawn, int team )
{
	bot->is_bot = true;
	
	// Use 'think' to pass the value of respawn to PutClientInServer.
	if( ! respawn )
	{
		bot->think = ACESP_HoldSpawn;
		bot->nextthink = level.framenum + random() * 3 * HZ;
	}
	else
	{
		#ifdef ACE_SPAWN
			bot->think = ACEAI_Think;
		#else
			bot->think = BOTAI_Think;
		#endif
		bot->nextthink = level.framenum + 1;
	}
	
	PutClientInServer( bot );
	
	JoinTeam( bot, team, true );
}

///////////////////////////////////////////////////////////////////////
// Respawn the bot
///////////////////////////////////////////////////////////////////////
void ACESP_Respawn (edict_t *self)
{
	respawn( self );
	
	if( random() < 0.05)
	{
		// Store current enemies available
		int		i, counter = 0;
		edict_t *myplayer[MAX_BOTS];

		if( self->lastkilledby)
		{
			// Have a comeback line!
			if(ltk_chat->value)	// Some people don't want this *sigh*
				LTK_Chat( self, self->lastkilledby, DBC_KILLED);
			self->lastkilledby = NULL;
		}
		else
		{
			// Pick someone at random to insult
			for(i=0;i<=num_players;i++)
			{
				// Find all available enemies to insult
				if(players[i] == NULL || players[i] == self || 
				   players[i]->solid == SOLID_NOT)
				   continue;
			
				if(teamplay->value && OnSameTeam( self, players[i]) )
				   continue;
				myplayer[counter++] = players[i];
			}
			if(counter > 0)
			{
				// Say something insulting to them!
				if(ltk_chat->value)	// Some people don't want this *sigh*
					LTK_Chat( self, myplayer[rand()%counter], DBC_INSULT);
			}
		}
	}	
}

///////////////////////////////////////////////////////////////////////
// Find a free client spot
///////////////////////////////////////////////////////////////////////
edict_t *ACESP_FindFreeClient (void)
{
	edict_t *bot = NULL;
	int i = 0;
	int max_count = 0;
	
	// This is for the naming of the bots
	for (i = game.maxclients; i > 0; i--)
	{
		bot = g_edicts + i + 1;
		
		if(bot->count > max_count)
			max_count = bot->count;
	}
	
	// Check for free spot
	for (i = game.maxclients; i > 0; i--)
	{
		bot = g_edicts + i + 1;

		if (!bot->inuse)
			break;
	}
	
	if (bot->inuse)
		return NULL;
	
	bot->count = max_count + 1; // Will become bot name...
	
	return bot;
}

///////////////////////////////////////////////////////////////////////
// Set the name of the bot and update the userinfo
///////////////////////////////////////////////////////////////////////
void ACESP_SetName(edict_t *bot, char *name, char *skin, char *team)
{
	float rnd;
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	char bot_name[MAX_INFO_STRING];

	if( (!name) || !strlen(name) )
	{
		// RiEvEr - new code to get random bot names
		LTKsetBotName(bot_name);
	}
	else
		strcpy(bot_name,name);

	// skin
	if( (!skin) || !strlen(skin) )
	{
		// randomly choose skin 
		rnd = random();
		if(rnd  < 0.05)
			sprintf(bot_skin,"male/bluebeard");
		else if(rnd < 0.1)
			sprintf(bot_skin,"female/leeloop");
		else if(rnd < 0.15)
			sprintf(bot_skin,"male/blues");
		else if(rnd < 0.2)
			sprintf(bot_skin,"female/sarah_ohconnor");
		else if(rnd < 0.25)
			sprintf(bot_skin,"actionmale/chucky");
		else if(rnd < 0.3)
			sprintf(bot_skin,"actionmale/axef");
		else if(rnd < 0.35)
			sprintf(bot_skin,"sas/sasurban");
		else if(rnd < 0.4)
			sprintf(bot_skin,"terror/urbanterr");
		else if(rnd < 0.45)
			sprintf(bot_skin,"aqmarine/urban");
		else if(rnd < 0.5)
			sprintf(bot_skin,"sydney/sydney");
		else if(rnd < 0.55)
			sprintf(bot_skin,"male/cajin");
		else if(rnd < 0.6)
			sprintf(bot_skin,"aqmarine/desert");
		else if(rnd < 0.65)
			sprintf(bot_skin,"male/grunt");
		else if(rnd < 0.7)
			sprintf(bot_skin,"male/mclaine");
		else if(rnd < 0.75)
			sprintf(bot_skin,"male/robber");
		else if(rnd < 0.8)
			sprintf(bot_skin,"male/snowcamo");
		else if(rnd < 0.85)
			sprintf(bot_skin,"terror/swat");
		else if(rnd < 0.9)
			sprintf(bot_skin,"terror/jungleterr");
		else if(rnd < 0.95)
			sprintf(bot_skin,"sas/saspolice");
		else 
			sprintf(bot_skin,"sas/sasuc");
	}
	else
		strcpy(bot_skin,skin);

	// initialise userinfo
	memset (userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	Info_SetValueForKey (userinfo, "name", bot_name);
	Info_SetValueForKey (userinfo, "skin", bot_skin);
	Info_SetValueForKey (userinfo, "hand", "2"); // bot is center handed for now!
//AQ2 ADD
	Info_SetValueForKey (userinfo, "spectator", "0"); // NOT a spectator
//AQ2 END

	ClientConnect (bot, userinfo);

//	ACESP_SaveBots(); // make sure to save the bots
}
//RiEvEr - new global to enable bot self-loading of routes
extern char current_map[55];
//

char *LocalTeamNames[4] = { "spectator", "1", "2", "3" };

///////////////////////////////////////////////////////////////////////
// Spawn the bot
///////////////////////////////////////////////////////////////////////
edict_t *ACESP_SpawnBot( char *team_str, char *name, char *skin, char *userinfo )
{
	int team = 0;
	edict_t	*bot = ACESP_FindFreeClient();
	if( ! bot )
	{
		gi.bprintf( PRINT_MEDIUM, "Server is full, increase Maxclients.\n" );
		return NULL;
	}
	
	bot->is_bot = true;
	bot->yaw_speed = 1000;  // deg/sec
	
	// To allow bots to respawn
	if( ! userinfo ) {
		if (ltk_classic->value) {
			// Classic naming method
			ACESP_SetName( bot, name, skin, team_str );  // includes ClientConnect
		} else if (!ltk_classic->value) {
			// TODO: New naming method
			ACESP_SetName( bot, name, skin, team_str );
		} else {
			ClientConnect( bot, userinfo );
		}
	}
	
	ClientBeginDeathmatch( bot );
	
	// Balance the teams!
	if( teamplay->value )
	{
		if( team_str )
			team = atoi( team_str );
		if( (team < TEAM1) || (team > teamCount) )
			team = GetNextTeamNumber();
		team_str = LocalTeamNames[ team ];
	}

	if(attract_mode->value && attract_mode_team->value){
		team = (int)attract_mode_team->value;
		if ((!use_3teams->value) && (team >= TEAM3)){
			gi.dprintf("Warning: attract_mode_team was %d, but use_3teams is not enabled!  Bots will default to team 1.\n", team);
			gi.cvar_forceset("attract_mode_team", "1");
			team = 1;
		}
	}
	
	ACESP_PutClientInServer( bot, true, team );
	
	ACEAI_PickLongRangeGoal(bot); // pick a new goal
	
	// LTK chat stuff
	if( random() < 0.33)
	{
		// Store current enemies available
		int		i, counter = 0;
		edict_t *myplayer[MAX_BOTS];

		for(i=0;i<=num_players;i++)
		{
			// Find all available enemies to insult
			if(players[i] == NULL || players[i] == bot || 
			   players[i]->solid == SOLID_NOT)
			   continue;
		
			if(teamplay->value && OnSameTeam( bot, players[i]) )
			   continue;
			myplayer[counter++] = players[i];
		}
		if(counter > 0)
		{
			// Say something insulting to them!
			if(ltk_chat->value)	// Some people don't want this *sigh*
				LTK_Chat( bot, myplayer[rand()%counter], DBC_WELCOME);
		}
	}	
	return bot;
}

void	ClientDisconnect( edict_t *ent );

///////////////////////////////////////////////////////////////////////
// Remove a bot by name or all bots
///////////////////////////////////////////////////////////////////////
void ACESP_RemoveBot(char *name)
{
	int i;
	qboolean freed=false;
	edict_t *bot;
	qboolean remove_all = (Q_stricmp(name,"all")==0) ? true : false;
	int find_team = (strlen(name)==1) ? atoi(name) : 0;

//	if (name!=NULL)
	for(i=0;i<game.maxclients;i++)
	{
		bot = g_edicts + i + 1;
		if(bot->inuse)
		{
			if( bot->is_bot && (remove_all || !strlen(name) || Q_stricmp(bot->client->pers.netname,name)==0 || (find_team && bot->client->resp.team==find_team)) )
			{
				bot->health = 0;
				player_die (bot, bot, bot, 100000, vec3_origin);
				// don't even bother waiting for death frames
//				bot->deadflag = DEAD_DEAD;
//				bot->inuse = false;
				freed = true;
				ClientDisconnect( bot );
//				ACEIT_PlayerRemoved (bot);
//				gi.bprintf (PRINT_MEDIUM, "%s removed\n", bot->client->pers.netname);
				if( ! remove_all )
					break;
			}
		}
	}

/*
	//Werewolf: Remove a random bot if no name given
	if(!freed)	
//		gi.bprintf (PRINT_MEDIUM, "%s not found\n", name);
	{
		do
		{
			i = (int)(rand()) % (int)(game.maxclients);
			bot = g_edicts + i + 1;
		}	while ( (!bot->inuse) || (!bot->is_bot) );
		bot->health = 0;
		player_die (bot, bot, bot, 100000, vec3_origin);
		freed = true;
		ClientDisconnect( bot );
	}
*/
	if(!freed)	
		gi.bprintf (PRINT_MEDIUM, "No bot removed\n", name);

//	ACESP_SaveBots(); // Save them again
}

void attract_mode_bot_check(void)
{
	int maxclientsminus2 = (int)(maxclients->value - 2);

	ACEIT_RebuildPlayerList();
	// Some sanity checking before we proceed

	// Cannot have the attract_mode_botcount at a value of N-2 of the maxclients
	if ((attract_mode->value == 2) && (attract_mode_botcount->value >= maxclientsminus2))
	{
		// If maxclients is 10 or more, set the botcount value to 6, else disable attract mode.  Increase your maxclients!
		if(maxclients->value >= 10){
			gi.dprintf( "attract_mode is 2, attract_mode_botcount is %d, maxclients is too low at %d, forcing it to default (6)\n", (int)attract_mode_botcount->value, (int)maxclients->value);
			gi.cvar_forceset("attract_mode_botcount", "6");
		} else {
			gi.dprintf( "attract_mode is 2, attract_mode_botcount is %d, maxclients is too low at %d, disabling attract_mode\n", (int)attract_mode_botcount->value, (int)maxclients->value);
			gi.cvar_forceset("attract_mode", "0");
		}
    }

	int tgt_bot_count = (int)attract_mode_botcount->value;
	int real_player_count = (num_players - game.bot_count);

	// Debug area, uncomment for gratiuitous amounts of spam
	// if (teamplay->value){
	// 	gi.dprintf("Team 1: %d - Team 2: %d, - Team 3: %d\n", team1, team2, team3);
	// }
	//gi.dprintf("tgt_bot_count is %d, real_player_count is %d, num_players is %d, game.bot_count is %d\n", tgt_bot_count, real_player_count, num_players, game.bot_count);
	// End debug area



	// Bot Maintenance
	/* Logic is as follows:
	  If (attract_mode_botcount - real_player_count) == game.botcount
	    (Current bots - real players is equal to attract_mode_botcount value)
	  Then do nothing, we are where we want to be

	  Else

	  If (attract_mode_botcount - real_player_count) > game.botcount
	  	(Current Bots + Real Players is less than the attract_mode_botcount value)
	  Then add a bot until these numbers are equal

	  Else

	  If (attract_mode_botcount - real_player_count) < game.botcount AND if attract_mode is 1
	  	(Current Bots + Real Players is more than the attract_mode_botcount value)
	  Then remove a bot until these numbers are equal

	  Else

	  If (total players == (maxclients - 1)) AND if attract_mode is 2
	  	(Current Bots + Real Players is more than the attract_mode_botcount value)
	  Then remove a bot only if we're near the maxclients number
	 
	*/
	
	if (tgt_bot_count - real_player_count == game.bot_count) {
		return;
	} else if (tgt_bot_count - real_player_count > game.bot_count) {
		//gi.dprintf("I'm adding a bot because %d - %d < %d", tgt_bot_count, real_player_count, game.bot_count);
		ACESP_SpawnBot(NULL, NULL, NULL, NULL);
	} else if ((tgt_bot_count - real_player_count < game.bot_count) && (attract_mode->value == 1)) {
		// This removes 1 bot per real player

		//gi.dprintf("I'm removing a bot because %d - %d > %d", tgt_bot_count, real_player_count, game.bot_count);
		ACESP_RemoveBot("");
	} else if ((num_players == maxclientsminus2) && (attract_mode->value == 2)) {
		// This removes 1 bot once we are at maxclients - 2 so we have room for a real player

		gi.dprintf("I'm removing a bot because num_players = %d and maxclients is %d", num_players, game.maxclients);
		ACESP_RemoveBot("");
	}
	
}

//====================================
// Stuff to generate pseudo-random names
//====================================
#define NUMNAMES	10
char	*names1[NUMNAMES] = {
	"Bad", "Death", "L33t", "Fast", "Real", "Lethal", "Hyper", "Hard", "Angel", "Red"};

char	*names2[NUMNAMES] = {
	"Moon", "evil", "master", "dude", "killa", "dog", "chef", "dave", "Zilch", "Amator" };

char	*names3[NUMNAMES] = {
	"Ana", "Bale", "Calen", "Cor", "Fan", "Gil", "Hali", "Line", "Male", "Pero"};

char	*names4[NUMNAMES] = {
	"ders", "rog", "born", "dor", "fing", "galad", "bon", "loss", "orch", "riel" };

qboolean	nameused[NUMNAMES][NUMNAMES];

//====================================
// AQ2World Staff Names -- come shoot at us!
// Find time to implement this!  Or better yet, 
// load names from a file rather than this array?
//====================================
#define AQ2WTEAM	11
#define AQ2WFULLNAME	AQ2WTEAM*2
char	*aq2names1[AQ2WTEAM] = {
	"bAron", "darksaint", "FragBait", "matic", "JukS", "TgT", "dmc", "dox", "KaniZ", "keffo", "QuimBy"
	};
char	*aq2names2[AQ2WTEAM] = {
	"Rezet", "Royce", "vrol", "mikota", "Reki", "ReKTeK", "Ralle", "Tech", "dmc", "Raptor007", "Ferrick"
	};
char	*aq2tags[AQ2WTEAM] = {
	"[BOT]", "<ai>", ".beep.", ">ROBOT<", "/iNhUmAn/", "^bZZZv,", "-machin3-", ")anDroiD)", "-b0rg_", "*r0b0t*", " FaK3:"
	};

qboolean	adminnameused[AQ2WTEAM][AQ2WFULLNAME];
// END AQ2World Staff Names //

void	LTKsetBotNameNew(void)
{
	return;
}

//====================================
// Classic random bot naming routine
//====================================
void	LTKsetBotName( char	*bot_name )
{
	int	part1,part2;
	int randomnames;
	part1 = part2 = 0;

	if(!attract_mode->value){
		randomnames = NUMNAMES;
	} else {
		randomnames = AQ2WTEAM;
	}

	while(1) {
			part1 = rand() % randomnames;
			part2 = rand() % randomnames;
			if (!attract_mode_newnames->value) {
				if (!nameused[part1][part2]) {
					break;
				}
			} else {
				if (!adminnameused[part1][part2]) {
					break;
				}
			}
		}

	// Mark that name as used
	if (!attract_mode_newnames->value) {
		nameused[part1][part2] = true;
	} else {
		adminnameused[part1][part2] = true;
	}
	// Now put the name together

	// Old names, for the classic feel
	if(!attract_mode_newnames->value) {
		if( random() < 0.5 )
		{
			strcpy( bot_name, names1[part1]);
			strcat( bot_name, names2[part2]);
		}
		else
		{
			strcpy( bot_name, names3[part1]);
			strcat( bot_name, names4[part2]);
		}
	} else { // New AQ2World Team names
		if( random() < 0.5 )
		{
			strcpy( bot_name, aq2names1[part1]);
			strcat( bot_name, aq2tags[part2]);
		}
		else
		{
			strcpy( bot_name, aq2tags[part1]);
			strcat( bot_name, aq2names2[part2]);
		}
	}
}

