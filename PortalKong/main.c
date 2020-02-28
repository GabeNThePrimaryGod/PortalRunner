//#define _CRT_SECURE_NO_WARNINGS
#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_font.h>

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <time.h>
#include <string.h>
#include "resources.h"

void main() {
	srand(clock());

	initProgram();

	ALLEGRO_SAMPLE *ambiance = al_load_sample("./assets/sons/ambiance/ambiance1.wav");		//C'EST GIGA CHIANT POUR LES TESTS
	al_play_sample(ambiance, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);

	player*P = initPlayer();

	tonneau*mob[nbMobMAX];
	tonneau*mob_boss[9][nbBossMobMAX];

	portal*portals[nbPortalMAX][2];

	sprite*player_runRight = initSprite("./assets/sprites/char/frame-%d.png", 72, 80, 6, 0);
	player_runRight->x = SCREENX / 3.5;
	player_runRight->y = SCREENY / 2;
	sprite*player_runLeft = initSprite("./assets/sprites/char/frame-%d.png", 72, 80, 6, 1);
	player_runLeft->x = SCREENX / 3.5;
	player_runLeft->y = SCREENY / 1.5;
	sprite*player_jump = initSprite("./assets/sprites/char/frame-%d.png", 64, 64, 6, 0);			//sprites lvl 0
	player_jump->x = SCREENX / 3.5;
	player_jump->y = SCREENY / 4;
	sprite*portal_orange = initSprite("./assets/sprites/portal_green/frame-%d.png", 48, 80, 6, 1);
	portal_orange->x = SCREENX / 1.7;
	portal_orange->y = SCREENY / 4.9;
	sprite*portal_blue = initSprite("./assets/sprites/portal_blue/frame-%d.png", 48, 80, 6, 1);
	portal_blue->x = SCREENX / 1.4;
	portal_blue->y = SCREENY / 4.9;

	//sprite*portal_tonneau1 = initSprite("./assets/sprites/portal_orange/frame-%d.png", 64, 64, 6, 0);		//sprites boules
	//sprite*portal_tonneau2 = initSprite("./assets/sprites/portal_orange/frame-%d.png", 64, 64, 6, 0);

	image*regles = initImage("./assets/regles/regles.png", SCREENX, SCREENY);
	//image*endOfGame = initImage("./assets/",SCREENX, SCREENY)
	ALLEGRO_BITMAP*endOfGame = al_create_bitmap(SCREENX, SCREENY);
	al_set_target_bitmap(endOfGame);
	al_clear_to_color(al_map_rgb(0, 0, 0));
	al_set_target_backbuffer(display);

	image*fond = initFond();

	image*level;
	int nbPortals;
	int fin;
	int endGame = false;
	bool level_is_loaded = false;
	int saut_regles = 0;
	bool isTonneaux;
	bool isBossTonneauxActivated;
	int curTextGlados = 0;
	bool showMenu = true;

	int player_ms = 0;
	int player_s = 0;
	int player_m = 0;
	int old_player_time;

	for (int i = 0; i < nbPortalMAX; i++)
		for (int j = 0; j < 2; j++) {
			if (!j)
				portals[i][j] = initPortal("./assets/sprites/portal_blue/frame-%d.png", 48, 80, 6);
			if (j)
				portals[i][j] = initPortal("./assets/sprites/portal_green/frame-%d.png", 48, 80, 6);
		}

	while (!level_is_loaded){
		if(showMenu){
			menu(&fin, &level_is_loaded);
			al_start_timer(timer);
			showMenu = false;
		}

		endGame = false;
		fin = 0;
		nbPortals = 0;
		nbTonneaux = 0;
		nbBossTonneaux = 0;
		isTonneaux = false;
		isBossTonneauxActivated = false;
		level = loadLevel(portals, &nbPortals, &isTonneaux, P);
		nbr_tonneaux_levels(curLevel);

		for (int i = 0; i < nbMobMAX; i++)			//Tonneaux
			mob[i] = initTonneau();
		for (int i = 0; i < 9; i++)
			for (int j = 0; j < nbBossMobMAX; j++) {
				mob_boss[i][j] = initTonneau();
				mob_boss[i][j]->dx = SCREENX;
				mob_boss[i][j]->dy = 300 + (32 * i);
				mob_boss[i][j]->spawnX = SCREENX;
				mob_boss[i][j]->spawnY = 300 + (32 * i);
				mob_boss[i][j]->depopX = SCREENX;
				mob_boss[i][j]->depopY = SCREENY;
				mob_boss[i][j]->move = 3;
			}

		level_is_loaded = true;
		printf("level %d sucessfuly loaded!\n", curLevel);
		if (curLevel == 1)
			al_start_timer(player_timer);
		
		// selon l'evenement
		
		while (!fin){
			al_get_keyboard_state(&key);
			al_wait_for_event(queue, &event);
			//routines calcul
			
			if (event.type == ALLEGRO_EVENT_TIMER) {

				////////////////////////// player timer
				if (al_get_timer_count(player_timer) > 0 && al_get_timer_count(player_timer) != old_player_time)
					player_ms++;
				if (player_ms >= 60) {
					player_ms = 0;
					player_s++;
					if (player_s >= 60) {
						player_s = 0;
						player_m++;
					}
				}
				old_player_time = al_get_timer_count(player_timer);

				/////////////////////////////////deplacements / animations
				movePlayer(P);
				if(nbPortals > 0)
					PortalsCollision(P, portals, nbPortals);

				///////////////////en fonction du niveau
				if (isTonneaux) {
					collisionJM(P, mob);
					//animSprite(portal_tonneau1);
					//animSprite(portal_tonneau2);
					
					addTonneau(1, vitesse_spawn);
					for (int i = 0; i < nbTonneaux; i++)
						moveTonneau(mob[i]);
				}
				if (curLevel == 6 && collision(P->x, P->y, 7)) {		//detection arrivée joueur salle boss
					isBossTonneauxActivated = true;
					nbBossTonneaux = 1;
				}
				if (curLevel == 6 && isBossTonneauxActivated) {		//tonneau boss
					add_bossTonneau(1, 100);		//400
					for (int i = 0; i < 9; i++)
						for (int j = 0; j < nbBossTonneaux; j++)
							move_bossTonneau(mob_boss[i][j]);
					collisionJM_bossTonneau(P, mob_boss);
				}
				if (curLevel == 7)
					if (al_get_timer_count(glados_timer) % 200 == 0 && curTextGlados + 1 < nbTextGlados) {		//defillement text Glados
						printf("curTextGlados : %d\n", curTextGlados);
						printf("%s\n", textGaldos[curTextGlados]);
						curTextGlados++;
					}

				/////////////////animation de tous les sprites tout les 5ms
				if (al_get_timer_count(timer) % 5 == 0) {		
					if (curLevel == 0) {
						animSprite(player_runRight);
						animSprite(player_runLeft);
						animSprite(player_jump);
						animSprite(portal_orange);
						animSprite(portal_blue);
					}
					for (int i = 0; i < nbPortals; i++)
						for (int j = 0; j < 2; j++)
							animPortal(portals[i][j]);
				}
			}
			
			//routines affichage
			if (al_is_event_queue_empty(queue)) {

				// nettoyage (afichage du fond)
				al_draw_bitmap(fond->img, 0, 0, 0);
				al_draw_bitmap(level->img, 0, 0, 0);

				for (int i = 0; i < nbPortals; i++)
					for (int j = 0; j < 2; j++)
						affichePortal(portals[i][j]);
				// affichage

				////////////////////////////////en fonction du niveau
				if (isTonneaux) {
					for (int i = 0; i < nbTonneaux; i++)
						afficheTonneau(mob[i]);
					//afficheSprite(portal_tonneau1);
					//afficheSprite(portal_tonneau2);
				}
				if (curLevel == 0) {		//tuto
					if (saut_regles == 0)
						saut_regles++;
					saut_regles = saut_menu(player_jump, saut_regles);
					al_draw_bitmap(regles->img, 0, 0, 0);
					afficheSprite(player_runRight);
					afficheSprite(player_runLeft);
					afficheSprite(player_jump);
					afficheSprite(portal_orange);
					afficheSprite(portal_blue);
				}
				if (curLevel == 6 && isBossTonneauxActivated) {		//tonneaux boss
					for (int i = 0; i < 9; i++)
						for (int j = 0; j < nbBossTonneaux; j++)
							afficheTonneau(mob_boss[i][j]);
				}
				if (curLevel == 6 && collision(P->x, P->y, 3))		//stop timer pour le dernier niveau
					al_stop_timer(player_timer);
				if (curLevel == 7) {
					affiche_textGlados(curTextGlados);
				
				}

				if (curLevel != 0) {
					aficheTimer("%d.%d..%d", player_m, player_s, player_ms * 1.65, SCREENX / 2, 10, al_map_rgb(233, 0, 255));
				}
				if (curLevel == 0) {
					affiche_textDebut();
				}
				affichePlayer(P);
				//copie buffer -> ecran
				al_flip_display();
			}

			if (curLevel == 7 && collision(P->x, P->y, 3)) {		//detection fin du jeu
				endGame = true;
				fin = 1;
			}
			/////////////////////////////////////////changement niveau
			if (collision(P->x, P->y, 3) && !fin || al_key_down(&key, ALLEGRO_KEY_F) && !fin) {		//changement Niveau
				if (al_key_down(&key, ALLEGRO_KEY_F)) {		//debug
					printf("enter a level to load :");
					scanf_s("%d", &curLevel);
				}
				else
					curLevel++;

				printf("loading level %d...\n", curLevel);

				al_destroy_bitmap(level->img);
				level_is_loaded = false;
				fin = 1;
				switch (curLevel) {
				case 0: copyTableau(&MAPDECOR, &level0);
					break;
				case 1: copyTableau(&MAPDECOR, &level1);
					break;
				case 2: copyTableau(&MAPDECOR, &level2);
					break;
				case 3: copyTableau(&MAPDECOR, &level3);
					break;
				case 4: copyTableau(&MAPDECOR, &level4);
					break;
				case 5: copyTableau(&MAPDECOR, &level5);
					break;
				case 6: copyTableau(&MAPDECOR, &level6);
					break;
				case 7: copyTableau(&MAPDECOR, &level7);
					al_start_timer(glados_timer);
				}
			}

			///////////////////////////////////////// fin du jeu	
			while (endGame) {
				al_get_keyboard_state(&key);
				//al_draw_bitmap(endOfGame, 0, 0, 0);
				al_clear_to_color(al_map_rgb(0, 0, 0));

				al_draw_text(font_2, al_map_rgb(233, 0, 255), 1, 1, ALLEGRO_ALIGN_LEFT,
					"ENTER : Recommencer");
				al_draw_text(font_2, al_map_rgb(233, 0, 255), SCREENX - 1, 1, ALLEGRO_ALIGN_RIGHT,
					"ESC : Quitter");

				al_draw_text(font_2, al_map_rgb(255, 255, 255), SCREENX / 2, (SCREENY / 4) * 1, ALLEGRO_ALIGN_CENTRE,
					"Bravo tu a reussi l'epreuve !");
				aficheTimer("ton score final est de : %d minutes %d secondes et %dms",
					player_m, player_s, player_ms * 1.65, SCREENX /2, (SCREENY / 4) * 2, al_map_rgb(233, 0, 255));
				
				/*al_draw_text(font_2, al_map_rgb(255, 255, 255), SCREENX / 2, (SCREENY / 6) * 3, ALLEGRO_ALIGN_CENTRE,
					"Ce n'est pas si mal, mais je pense que tu peux mieux faire ! ");
				al_draw_text(font_2, al_map_rgb(255, 255, 255), SCREENX / 2, (SCREENY / 6) * 4, ALLEGRO_ALIGN_CENTRE,
					"Sache que tu peux reessayer autant de fois que tu le desires");
				al_draw_text(font_2, al_map_rgb(255, 255, 255), SCREENX / 2, (SCREENY / 6) * 5, ALLEGRO_ALIGN_CENTRE,
					"je me ferais un plaisir de t'observer !");*/
				
				al_flip_display();
				if (al_key_down(&key, ALLEGRO_KEY_ENTER)) {
					curLevel = 0;
					player_ms = 0;
					player_s = 0;
					player_m = 0;
					curTextGlados = 0;
					isBossTonneauxActivated = false;
					old_player_time = NULL;
					copyTableau(&MAPDECOR, &level0);
					showMenu = true;
					fin = 1;
					level_is_loaded = false;
					endGame = false;
					nbr_tonneaux_levels(curLevel);
				}
				if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE || al_key_down(&key, ALLEGRO_KEY_ESCAPE)){
					fin = 1;
					endGame = false;
				}
			}
			if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE || al_key_down(&key, ALLEGRO_KEY_ESCAPE))
				fin = 1;
		}
	}
	al_destroy_display(display);
}
