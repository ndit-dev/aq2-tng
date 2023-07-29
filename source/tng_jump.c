#include "g_local.h"

//cvar_t *jump;

char *jump_statusbar =
    "yb -24 "
    "if 9 "
    "xr 0 "
    "yb 0 "
    "xv 0 "
    "yv 0 "
    "pic 9 "
    "endif "
    //  Current Speed
    "xr -105 "
    "yt 2 "
    "string \"Current Speed\" "
    "xr -82 "
    "yt 12 "
    "num 6 1 "

    //  High Speed
    "xr -81 "
    "yt 34 "
    "string \"High Speed\" "
    "xr -82 "
    "yt 44 "
    "num 6 2 "

    //  last fall damage
    "xr -129 "
    "yt 66 "
    "string \"Last Fall Damage\" "
    "xr -82 "
    "yt 76 "
    "num 10 3 "
;

void Jmp_SetStats(edict_t *ent)
{
	vec3_t	velocity;
	vec_t	speed;

	// calculate speed
	VectorClear(velocity);
	VectorCopy(ent->velocity, velocity);
	speed = VectorNormalize(velocity);

	if(speed > ent->client->resp.jmp_highspeed)
		ent->client->resp.jmp_highspeed = speed;

	ent->client->ps.stats[STAT_SPEEDX] = speed;
	ent->client->ps.stats[STAT_HIGHSPEED] = ent->client->resp.jmp_highspeed;
	ent->client->ps.stats[STAT_FALLDMGLAST] = ent->client->resp.jmp_falldmglast;

	//
	// layouts
	//
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (ent->health <= 0 || level.intermission_framenum || ent->client->layout)
		ent->client->ps.stats[STAT_LAYOUTS] |= 1;
	if (ent->client->showinventory && ent->health > 0)
		ent->client->ps.stats[STAT_LAYOUTS] |= 2;

	SetIDView (ent);
}

void Jmp_EquipClient(edict_t *ent)
{
	memset(ent->client->inventory, 0, sizeof(ent->client->inventory));
	ent->client->weapon = 0;

	// make client non-solid
	ent->solid = SOLID_TRIGGER;
	AddToTransparentList(ent);
}

void Cmd_Jmod_f (edict_t *ent)
{
	char *cmd = NULL;

	if( ! jump->value )
	{
		gi.cprintf(ent, PRINT_HIGH, "The server does not have JumpMod enabled.\n");
		return;
	}

	if(gi.argc() < 2) {
		gi.cprintf(ent, PRINT_HIGH, "AQ2:TNG Jump mode commands:\n");
		gi.cprintf(ent, PRINT_HIGH, " jmod store - save your current point\n");
		gi.cprintf(ent, PRINT_HIGH, " jmod recall - teleport back to saved point\n");
		gi.cprintf(ent, PRINT_HIGH, " jmod reset - remove saved point\n");
		gi.cprintf(ent, PRINT_HIGH, " jmod clear - reset stats\n");
		return;
	}

	cmd = gi.argv(1);

	if(Q_stricmp(cmd, "store") == 0)
	{
		Cmd_Store_f(ent);
		return;
	}
	else if(Q_stricmp(cmd, "recall") == 0)
	{
		Cmd_Recall_f(ent);
		return;
	}
	else if(Q_stricmp(cmd, "reset") == 0)
	{
		Cmd_Reset_f(ent);
		return;
	}
	else if(Q_stricmp(cmd, "clear") == 0 || Q_stricmp(cmd, "rhs") == 0)
	{
		Cmd_Clear_f(ent);
		return;
	}
	else if(Q_stricmp(cmd, "goto") == 0)
	{
		Cmd_Goto_f(ent);
		return;
	}
	else if(Q_stricmp(cmd, "spawnp") == 0)
	{
		Cmd_GotoP_f(ent);
		return;
	}
	else if(Q_stricmp(cmd, "spawnc") == 0)
	{
		Cmd_GotoPC_f(ent);
		return;
	}
	else if(Q_stricmp(cmd, "lca") == 0)
	{
		Cmd_PMLCA_f(ent);
		return;
	}
	else if(Q_stricmp(cmd, "noclip") == 0)
	{
		Cmd_PMNoclip_f(ent);
		return;
	}

	gi.cprintf(ent, PRINT_HIGH, "Unknown jmod command\n");
}

edict_t *SelectClosestDeathmatchSpawnPoint (void)
{
	edict_t *bestspot;
	float   bestdistance, bestplayerdistance;
	edict_t *spot;

	spot = NULL;
	bestspot = NULL;
	bestdistance = -1;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
			bestplayerdistance = PlayersRangeFromSpot (spot);
			if ((bestplayerdistance < bestdistance) || (bestdistance < 0))
			{
					bestspot = spot;
					bestdistance = bestplayerdistance;
			}
	}
	return bestspot;
}

void Cmd_PMLCA_f(edict_t *ent)
{
	if (ent->client->pers.spectator)
	{
		gi.cprintf(ent,PRINT_HIGH,"This command cannot be used by spectators\n");
		ent->client->resp.toggle_lca = 0;
		return;
	}

	if (!ent->client->resp.toggle_lca)
	{
		gi.centerprintf (ent,"LIGHTS...\n");
		gi.sound(ent, CHAN_VOICE, gi.soundindex("atl/lights.wav"), 1, ATTN_STATIC, 0);
		ent->client->resp.toggle_lca = 41;
	}
	else if (ent->client->resp.toggle_lca == 21)
	{
		gi.centerprintf (ent,"CAMERA...\n");
		gi.sound(ent, CHAN_VOICE, gi.soundindex("atl/camera.wav"), 1, ATTN_STATIC, 0);
	}
    else if (ent->client->resp.toggle_lca == 1)
	{
		gi.centerprintf (ent,"ACTION!\n");
		gi.sound(ent, CHAN_VOICE, gi.soundindex("atl/action.wav"), 1, ATTN_STATIC, 0);
	}
	ent->client->resp.toggle_lca--;
}

edict_t *PMSelectSpawnPoint (int number)
{
        edict_t *spot;
        int             count = 0;
        int             selection;

        spot = NULL;
        while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
			count++;

        if (!count)
			return NULL;

		//if random was selected, pick one
		if (number == 0) {
	        selection = rand() % count + 1;
		} else {
			//if person wanted tele 7 but only 5 found, set to 5
			if (number > count)
				number = count;
			//if person input a negative, set to 0
			else if (number < 0)
				number = 0;
			selection = number;
		}

		spot = NULL;
		do
		{
			spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
			selection--;
		} while (selection > 0);

        return spot;
}

void Cmd_Goto_f (edict_t *ent)
{
	int 		i;
	char		*s, *token;
	vec3_t		teleport_goto;

	if (!ent->deadflag && !ent->client->pers.spectator)
	{
		if (gi.argc() == 4)
		{
			s = strdup(gi.args());

			i=0;
			token = strtok( s, " " );

			while( token != NULL )
			{
				teleport_goto[i] = 0;
				teleport_goto[i] = atoi(token);
				token = strtok( NULL, " " );
				i++;
			}
			teleport_goto[2] -= ent->viewheight;

			ent->client->jumping = 0;
			ent->movetype = MOVETYPE_NOCLIP;

			gi.unlinkentity (ent);

			VectorCopy (teleport_goto, ent->s.origin);
			VectorCopy (teleport_goto, ent->s.old_origin);

			// clear the velocity and hold them in place briefly
			VectorClear (ent->velocity);

			ent->client->ps.pmove.pm_time = 160>>3;		// hold time

			// draw the teleport splash on the player
			ent->s.event = EV_PLAYER_TELEPORT;

			VectorClear (ent->s.angles);
			VectorClear (ent->client->ps.viewangles);
			VectorClear (ent->client->v_angle);

			gi.linkentity (ent);
			
			ent->movetype = MOVETYPE_WALK;
		}
		else
			gi.cprintf(ent,PRINT_HIGH,"Wrong syntax: goto <#> <#> <#>\n");
	}
	else
		gi.cprintf(ent,PRINT_HIGH,"This command cannot be used by spectators\n");
	
}

void Cmd_GotoP_f (edict_t *ent)
{
	vec3_t		teleport_goto, angles;
	edict_t 	*spot = NULL;
	char 		*buffer="\0";

	if (!ent->deadflag && !ent->client->pers.spectator)
	{
		int i;
		if (gi.argc() > 1)
		{
			buffer = strtok(gi.args()," ");
			spot = PMSelectSpawnPoint(atoi(buffer));
		}
		else
			spot = PMSelectSpawnPoint(0);

        VectorCopy (spot->s.origin, teleport_goto);
        teleport_goto[2] += 9;
        VectorCopy (spot->s.angles, angles);

		ent->client->jumping = 0;
		ent->movetype = MOVETYPE_NOCLIP;
		gi.unlinkentity (ent);
		
		VectorCopy (teleport_goto, ent->s.origin);
		VectorCopy (teleport_goto, ent->s.old_origin);
		
		// clear the velocity and hold them in place briefly
		VectorClear (ent->velocity);
		
		ent->client->ps.pmove.pm_time = 160>>3;		// hold time
		//ent->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;
		
		// draw the teleport splash on the player
		ent->s.event = EV_PLAYER_TELEPORT;
		
		VectorClear (ent->s.angles);
		VectorClear (ent->client->ps.viewangles);
		VectorClear (ent->client->v_angle);

		VectorCopy(angles,ent->s.angles);
		VectorCopy(ent->s.angles,ent->client->v_angle);

		for (i=0;i<2;i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->v_angle[i] - ent->client->resp.cmd_angles[i]);
		if (ent->client->pers.spectator)
			ent->solid = SOLID_BBOX;
		else
			ent->solid = SOLID_TRIGGER;
		
		ent->deadflag = DEAD_NO;

		gi.linkentity (ent);

		ent->movetype = MOVETYPE_WALK;
	}
	else
		gi.cprintf(ent,PRINT_HIGH,"This command cannot be used by spectators\n");
}

void Cmd_GotoPC_f (edict_t *ent)
{
	vec3_t		teleport_goto, angles;
	edict_t 	*spot = NULL;

	if (!ent->deadflag && !ent->client->pers.spectator)
	{
		int i;
		spot = SelectClosestDeathmatchSpawnPoint();

        VectorCopy (spot->s.origin, teleport_goto);
        teleport_goto[2] += 9;
        VectorCopy (spot->s.angles, angles);

		ent->client->jumping = 0;
		ent->movetype = MOVETYPE_NOCLIP;
		gi.unlinkentity (ent);

		VectorCopy (teleport_goto, ent->s.origin);
		VectorCopy (teleport_goto, ent->s.old_origin);

		// clear the velocity and hold them in place briefly
		VectorClear (ent->velocity);

		ent->client->ps.pmove.pm_time = 160>>3;		// hold time
		//ent->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;
		
		// draw the teleport splash on the player
		ent->s.event = EV_PLAYER_TELEPORT;
		
		VectorClear (ent->s.angles);
		VectorClear (ent->client->ps.viewangles);
		VectorClear (ent->client->v_angle);

		VectorCopy(angles,ent->s.angles);
		VectorCopy(ent->s.angles,ent->client->v_angle);

		for (i=0;i<2;i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->v_angle[i] - ent->client->resp.cmd_angles[i]);
		
		if (ent->client->pers.spectator)
			ent->solid = SOLID_BBOX;
		else
			ent->solid = SOLID_TRIGGER;
		
		ent->deadflag = DEAD_NO;
		
		gi.linkentity (ent);
		
		ent->movetype = MOVETYPE_WALK;
	}
	else
		gi.cprintf(ent,PRINT_HIGH,"This command cannot be used by spectators\n");
}

void Cmd_Clear_f(edict_t *ent)
{
	ent->client->resp.jmp_highspeed = 0;
	ent->client->resp.jmp_falldmglast = 0;
	gi.cprintf(ent, PRINT_HIGH, "Statistics cleared\n");
}

void Cmd_Reset_f (edict_t *ent)
{
	VectorClear(ent->client->resp.jmp_teleport_origin);
	VectorClear(ent->client->resp.jmp_teleport_v_angle);
	gi.cprintf(ent, PRINT_HIGH, "Teleport location removed\n");
}

void Cmd_Store_f (edict_t *ent)
{
	if (ent->client->pers.spectator)
	{
		gi.cprintf(ent, PRINT_HIGH, "This command cannot be used by spectators\n");
		return;
	}

	VectorCopy (ent->s.origin, ent->client->resp.jmp_teleport_origin);
	VectorCopy(ent->client->v_angle, ent->client->resp.jmp_teleport_v_angle);

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->client->resp.jmp_teleport_ducked = true;
	}

	gi.cprintf(ent, PRINT_HIGH, "Location stored\n");
}

void Cmd_Recall_f (edict_t *ent)
{
	int i;

	if (ent->deadflag || ent->client->pers.spectator)
	{
		gi.cprintf(ent, PRINT_HIGH, "This command cannot be used by spectators or dead players\n");
		return;
	}

	if(VectorLength(ent->client->resp.jmp_teleport_origin) == 0)
	{
		gi.cprintf(ent, PRINT_HIGH, "You must first \"store\" a location to teleport to\n");
		return;
	}

	ent->client->jumping = 0;

	ent->movetype = MOVETYPE_NOCLIP;

	/* teleport effect */
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TELEPORT_EFFECT);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.unlinkentity (ent);

	VectorCopy (ent->client->resp.jmp_teleport_origin, ent->s.origin);
	VectorCopy (ent->client->resp.jmp_teleport_origin, ent->s.old_origin);

	VectorClear (ent->velocity);

	ent->client->ps.pmove.pm_time = 160>>3;

	ent->s.event = EV_PLAYER_TELEPORT;

	VectorClear(ent->s.angles);
	VectorClear(ent->client->ps.viewangles);
	VectorClear(ent->client->ps.kick_angles);
	VectorClear(ent->client->v_angle);
	VectorClear(ent->client->ps.pmove.delta_angles);
	VectorClear(ent->client->kick_angles);
	ent->client->fall_time = 0;
	ent->client->fall_value = 0;

	VectorCopy(ent->client->resp.jmp_teleport_v_angle, ent->client->v_angle);
	VectorCopy(ent->client->v_angle, ent->client->ps.viewangles);

	for (i=0;i<2;i++)
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->v_angle[i] - ent->client->resp.cmd_angles[i]);

	if (ent->client->resp.jmp_teleport_ducked)
		ent->client->ps.pmove.pm_flags = PMF_DUCKED;

	gi.linkentity (ent);

	/* teleport effect */
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TELEPORT_EFFECT);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->movetype = MOVETYPE_WALK;
}

static void Cmd_PMNoclip_f (edict_t * ent)
{
	char *msg;

	if (ent->movetype == MOVETYPE_NOCLIP)
	{
		ent->movetype = MOVETYPE_WALK;
		msg = "noclip OFF\n";
	}
	else
	{
		ent->movetype = MOVETYPE_NOCLIP;
		msg = "noclip ON\n";
	}

	gi.cprintf (ent, PRINT_HIGH, msg);
}

// void Puppet_Spawn (edict_t *self,qboolean skip,qboolean nomsg)
// {
// 	edict_t *puppet = NULL;	

// 	if (!skip)
// 	{
// 		if (self->client->pers.spectator)
// 		{
// 			if (self->client->puppet)
// 			{
// 				G_FreeEdict(self->client->puppet);
// 				self->client->puppet = NULL;
// 				self->client->resp.puppetdemo_state = PUPPET_NEW_NOTHING;
// 				return; //avoid the message if they just went spec
// 			}
// 			if (!nomsg)
// 				gi.cprintf(self,PRINT_HIGH,"This command cannot be used to spawn a puppet when spectating\n");
// 			return;
// 		}
// 	}

// 	if (self->client->puppet)
// 	{  
// 		G_FreeEdict(self->client->puppet);
// 		self->client->puppet = NULL;

// 		if (self->client->resp.puppetdemo_state == PUPPET_NEW_RECORDING)
// 			self->client->resp.puppetdemo_pup_recorded = false;
// 		else
// 			self->client->resp.puppetdemo_state = PUPPET_NEW_NOTHING;
// 		if (!nomsg)
// 			gi.centerprintf (self,"Puppet removed\n");
// 		return;
// 	}

// 	puppet = G_Spawn ();
// 	puppet->classname = "puppet";
// 	puppet->classnum = 0;
// 	puppet->takedamage = DAMAGE_AIM;
// 	puppet->movetype = MOVETYPE_TOSS;
	
// 	puppet->solid = SOLID_TRIGGER;
// 	puppet->deadflag = DEAD_NO;
// 	puppet->clipmask = (CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW);

// 	if (self->client->pers.spectator)
// 	{
// 		puppet->model = "players/male/tris.md2";
// 		puppet->s.modelindex = 255;
// 		puppet->s.modelindex2 = 255;
// 		puppet->s.skinnum = 256;
// 	}
// 	else
// 	{
// 		puppet->model = self->model;
// 		puppet->s.modelindex = 255;
// 		puppet->s.modelindex2 = self->s.modelindex2;
// 		puppet->s.skinnum |= self->s.skinnum;
// 	}

// 	// if (self->client->resp.wmodes & OS_PUPPET_TRANS)
// 	// 	puppet->s.renderfx |= RF_TRANSLUCENT;
// 	// else
// 	puppet->s.renderfx = 0;
	   
// 	puppet->think = Puppet_Think;
// 	puppet->nextthink = level.time + .1;
// 	puppet->owner = world;
// 	puppet->owner = self;
// 	puppet->viewheight = 22;
// 	VectorSet (puppet->mins, -16, -16, -24);
// 	VectorSet (puppet->maxs, 16, 16, 32);

// 	if (self->movetype == MOVETYPE_NOCLIP)
// 	{
// 		puppet->maxs[2] = 32;
// 		puppet->s.frame = 0;
// 	}
// 	else
// 	{
// 		puppet->maxs[2] = self->maxs[2];
// 		puppet->s.frame = self->s.frame;
// 	}

// 	if (!self->client->pers.spectator)
// 	{
// 		VectorCopy(self->s.origin,puppet->s.origin);
// 		puppet->s.angles[1] = self->s.angles[1];
// 	}

// 	VectorClear (puppet->velocity);

// 	puppet->movetarget = NULL;

// 	self->client->puppet = puppet;
// 	self->client->resp.puppetdemo_pause = false;
// 	self->client->resp.puppetdemo_frame = 0;

// 	gi.linkentity(puppet);
// 	AddToTransparentList(puppet);

// 	if (!nomsg)
// 		gi.centerprintf (self,"Puppet created\n");
// }
