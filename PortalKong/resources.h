#include "levels.h"

#define TILEX 16
#define TILEY 16

#define nbPortalMAX 6
#define nbMobMAX 70
#define nbBossMobMAX 20

typedef struct player {
	int x, y;
	int h, w;
	int tx, ty;
	int curFrame;
	int flags;
	ALLEGRO_BITMAP*frames[7];
}player;
typedef struct sprite {
	int x, y;
	int h, w;
	int tx, ty;
	int curFrame;
	int size;
	int flags;
	ALLEGRO_BITMAP*frames[30];
}sprite;
typedef struct image {
	int w, h;
	int tx, ty;
	ALLEGRO_BITMAP*img;
}image;
typedef struct level {
	int w, h;
	int tx, ty;			// a voir
	ALLEGRO_BITMAP*img;
}level;
typedef struct tonneau {
	int centerx, centery;
	int dx, dy;
	int spawnX, spawnY;
	int depopX, depopY;
	float angle;
	int move, pat;
	ALLEGRO_BITMAP*img;
}tonneau;
typedef struct portal {
	int x, y;
	int desx, desy;
	int h, w;
	int tx, ty;
	int curFrame;
	int size;
	ALLEGRO_BITMAP*frames[30];
}portal;
typedef struct piece {
	int x, y;
	int h, w;
	int tx, ty;
	bool isClamed;
	ALLEGRO_BITMAP*img;
}piece;

//tools
void copyTableau(int*tab1[][MAPTX], int*tab2[][MAPTX]);
void beurre(const char*txt);
int is_key_pressed(ALLEGRO_KEYBOARD_STATE*key, int touche, int repeat);

//init
void initProgram(void);
void menu(int*fin, bool*level_is_loaded);
image*initFond(void);
image*loadLevel(portal*portals[][2], int*nbPortals, bool*isTonneaux, player*p);
void nbr_tonneaux_levels(int curlevel);

player*initPlayer(void);
sprite*initSprite(char*path, int tx, int ty, int size, int flags);
portal*initPortal(char*path, int tx, int ty, int size);
tonneau*initTonneau(void);
image*initImage(char*path, int x, int y);

//deplace
int saut_menu(sprite*pl, int saut_regles);
void movePlayer(player*p);
void moveTonneau(tonneau*mob);
void addTonneau(int nb, int delay);
void animSprite(sprite*s);
void animPortal(portal*port);

//collision
bool collision(int x, int y, int nbTile);
void PortalsCollision(player*p, portal*portals[][2], int*nbPortals);
void collisionJM(player*p, tonneau*mob[]);

//affiche
void affichePlayer(player*p);
void afficheSprite(sprite*s);
void affichePortal(portal*port);
void afficheTonneau(tonneau*mob);
void aficheTimer(const char*txt ,int m, int s, int ms, int x, int y, ALLEGRO_COLOR color);

void afficheImage(image*i, int x, int y, int flags);

//BOSS ONLY
void add_bossTonneau(int nb, int delay);
void move_bossTonneau(tonneau*mob);
void collisionJM_bossTonneau(player*p, tonneau*mob[9][nbBossMobMAX]);
void affiche_textGlados(int curTextGlados);

//debug
void drawMatrix(void);
void drawPortalsHitBox(portal*portals[][2]);
void drawBoulesHitBox(tonneau*mob[]);

ALLEGRO_EVENT_QUEUE*queue;
ALLEGRO_KEYBOARD_STATE key;
ALLEGRO_DISPLAY*display;
//ALLEGRO_TEXTLOG*debugConsole; // debug
ALLEGRO_EVENT event;
ALLEGRO_TIMER*timer;
ALLEGRO_TIMER*player_timer;
ALLEGRO_TIMER*glados_timer;

ALLEGRO_FONT*font_1;	//police 8 bits
ALLEGRO_FONT*font_2;	//police arial
ALLEGRO_FONT*font_3;	//police arial petit (pour GLAdOS)

int SCREENX = MAPTX * TILEX;
int SCREENY = MAPTY * TILEY;

int XspawnTonneaux = -99;
int YspawnTonneaux = -99;		//valeurs par default
int XdepopTonneaux = -99;
int YdepopTonneaux = -99;		//valeurs par default

int vitesse_spawn = 50;		//vitesse spawn tonneaux par default

int nbTonneauxMAX = 10;

int curLevel = 0;		//niveau actuel

int nbTonneaux = 0;
int nbBossTonneaux = 0;
int boule_touche = 0;
int boss_boule_touche = 0;
int tp_CD = 0;			//temporaire
int saut = 0;

/*********************************************
INIT
**********************************************/
void initProgram(void) {
	SetConsoleOutputCP(1252);
	al_init();
	al_install_keyboard();
	al_init_primitives_addon();

	//create display
	al_set_new_display_flags(ALLEGRO_WINDOWED |
		ALLEGRO_RESIZABLE);
	display = al_create_display(SCREENX, SCREENY);
	al_set_window_title(display, "Portal Runer");
	//debug
	//debugConsole = al_open_native_text_log("debugConsole", 0);

//images
	al_init_image_addon();

	//timers
	timer = al_create_timer(1.0 / 60);		//initialisation timer		
	player_timer = al_create_timer(1.0 / 60);		//initialisation player timer
	glados_timer = al_create_timer(1.0 / 60);		//initialisation player timer

	//Event Queue
	queue = al_create_event_queue();

	//ecritures
	al_init_font_addon();
	al_init_ttf_addon();
	font_1 = al_load_font("./assets/fonts/8-BIT WONDER.TTF", 36, NULL);
	font_2 = al_load_font("./assets/fonts/arial_narrow_7.ttf", 45, NULL);
	font_3 = al_load_font("./assets/fonts/arial_narrow_7.ttf", 20, NULL);

	al_install_audio();
	al_init_acodec_addon();
	al_reserve_samples(1);

	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_timer_event_source(timer));
	al_register_event_source(queue, al_get_display_event_source(display));
}
void menu(int*fin, bool*level_is_loaded) {

	ALLEGRO_TIMER*timerMenu;
	timerMenu = al_create_timer(1.0 / 60);
	al_start_timer(timerMenu);

	ALLEGRO_BITMAP*menu;
	menu = al_load_bitmap("./assets/menu/menu.png");

	ALLEGRO_BITMAP*menu_start;
	menu_start = al_load_bitmap("./assets/menu/text.png");
	al_set_target_backbuffer(display);

	do {
		al_get_keyboard_state(&key);
		if (al_get_timer_count(timerMenu) % 100 >= 0 && al_get_timer_count(timerMenu) % 100 <= 20)
			al_draw_scaled_bitmap(menu, 0, 0, 1920, 1080, 1, 1, SCREENX, SCREENY, 0);

		if (al_get_timer_count(timerMenu) % 100 >= 21 && al_get_timer_count(timerMenu) % 100 <= 100)
			al_draw_text(font_1, al_map_rgb(0, 70, 190), SCREENX / 2, SCREENY / 3 + TILEY * 2, ALLEGRO_ALIGN_CENTRE, "Press SpaceBar to Start");

		al_flip_display();
		if (al_key_down(&key, ALLEGRO_KEY_SPACE))
			break;
		if (al_key_down(&key, ALLEGRO_KEY_ESCAPE)) {
			*fin = 1;
			*level_is_loaded = true;
			al_stop_timer(timerMenu);
			return;
		}
	} while (1);

	int select = 0;
	//int select_CD = 0;
	bool endMenu = false;
	bool endSelector = false;
	do {			//boucle menu
		do {		//boucle selecteur
			al_get_keyboard_state(&key);
			//if (select_CD > 0 && al_get_timer_count(timerMenu) % 5 == 0)
				//select_CD--;

			//if (select_CD == 0) {
				if (al_key_down(&key, ALLEGRO_KEY_UP) && select > 0) {
					select--;
					//select_CD = 15;
				}
				if (al_key_down(&key, ALLEGRO_KEY_DOWN) && select < 1) {
					select++;
					//select_CD = 15;
				}
			//}

			al_draw_scaled_bitmap(menu, 0, 0, 1920, 1080, 1, 1, SCREENX, SCREENY, 0);
			al_draw_text(font_1, al_map_rgb(233, 0, 255), SCREENX / 2, SCREENY / 3, ALLEGRO_ALIGN_CENTRE, "Lancer le jeu");
			al_draw_text(font_1, al_map_rgb(233, 0, 255), SCREENX / 2, SCREENY / 2, ALLEGRO_ALIGN_CENTRE, "Credits");

			switch (select) {
			case 0:al_draw_text(font_1, al_map_rgb(255, 255, 255), SCREENX / 2, SCREENY / 3, ALLEGRO_ALIGN_CENTRE, "Lancer le jeu");
				break;
			case 1:al_draw_text(font_1, al_map_rgb(255, 255, 255), SCREENX / 2, SCREENY / 2, ALLEGRO_ALIGN_CENTRE, "Credits");
			}

			//al_draw_triangle(0,0,0,100,100,50, al_map_rgb(255, 255, 255), 1);
			
			al_flip_display();
			if (al_key_down(&key, ALLEGRO_KEY_ENTER) && select > -1 && select < 2)
				endSelector = true;
			if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE || al_key_down(&key, ALLEGRO_KEY_ESCAPE)) {
				*fin = 1;
				*level_is_loaded = true;
				al_stop_timer(timerMenu);
				return;
			}
		} while (!endSelector);
		if (select == 0)
			return;
		while (select == 1) {
			al_get_keyboard_state(&key);
			al_clear_to_color(al_map_rgb(0, 0, 0));
			al_draw_text(font_3, al_map_rgb(255, 255, 255), SCREENX / 2, SCREENY / 2.6, ALLEGRO_ALIGN_CENTRE, "Portal Runner a ete programme en C grace a la librairie ALLEGRO 5");
			al_draw_text(font_3, al_map_rgb(255, 255, 255), SCREENX / 2, SCREENY / 2.4, ALLEGRO_ALIGN_CENTRE, "par Jessica FAUCHET, Florian AUROUSSEAU, Robin ZMUDA");
			al_draw_text(font_3, al_map_rgb(255, 255, 255), SCREENX / 1.02, SCREENY / 1.05, ALLEGRO_ALIGN_RIGHT, "BACKSPACE pour revenir au menu");
			if (al_key_down(&key, ALLEGRO_KEY_BACKSPACE)) {
				endMenu = false;
				endSelector = false;
				break;
			}
			al_flip_display();
		}
	} while (!endMenu);
	al_stop_timer(timerMenu);


}
image*initFond(void) {
	image*f = (image*)malloc(sizeof(image));
	f->img = al_create_bitmap(SCREENX, SCREENY);

	al_set_target_bitmap(f->img);
	al_clear_to_color(al_map_rgb(220, 220, 220));

	for (int x = 0; x < SCREENX; x += 50)
		for (int y = 1; y < SCREENY; y++)
			al_put_pixel(x, y, al_map_rgb(100, 100, 100));

	for (int y = 0; y < SCREENY; y += 70)
		for (int x = 1; x < SCREENX; x++)
			al_put_pixel(x, y, al_map_rgb(100, 100, 100));
	al_set_target_backbuffer(display);
	f->tx = SCREENX;
	f->ty = SCREENY;
	f->h = SCREENX;
	f->w = SCREENY;
	return f;
}
image*loadLevel(portal*portals[][2], int*nbPortals, bool*isTonneaux, player*p) 
{
	image*l = (image*)malloc(sizeof(image));

	l->img = al_create_bitmap(SCREENX, SCREENY);
	ALLEGRO_BITMAP*tile_0 = al_load_bitmap("./assets/level/0.png");
	image*glados_1 = initImage("./assets/level/1.png", TILEX * 8, TILEY * 8);
	image*entre_2 = initImage("./assets/level/2.png", TILEX * 2, TILEY * 2);
	image*sortie_3 = initImage("./assets/level/3.png", TILEX * 2, TILEY * 2);
	image*gateau_4 = initImage("./assets/level/4.png", TILEX * 2, TILEY * 2);

	int found_count[nbPortalMAX];
	for (int i = 0; i < nbPortalMAX; i++)
		found_count[i] = -1;

	for (int mapy = 0; mapy < MAPTY; mapy++)		//draw tiles
		for (int mapx = 0; mapx < MAPTX; mapx++) 
		{
			al_set_target_bitmap(l->img);

			if (MAPDECOR[mapy][mapx] == 0)
				al_draw_bitmap(tile_0, mapx * TILEX, mapy * TILEY, 0);

			if (MAPDECOR[mapy][mapx] == 1){
				if (curLevel == 7)
					al_draw_scaled_bitmap(glados_1->img, 0, 0, glados_1->w, glados_1->h, mapx * TILEX, mapy * TILEY, glados_1->tx, glados_1->ty, 1);
				else 
					al_draw_scaled_bitmap(glados_1->img, 0, 0, glados_1->w, glados_1->h, mapx * TILEX, mapy * TILEY, glados_1->tx, glados_1->ty, 0);
			}

			if (MAPDECOR[mapy][mapx] == 2) {
				al_draw_scaled_bitmap(entre_2->img, 0, 0, entre_2->w, entre_2->h, mapx * TILEX, mapy * TILEY, entre_2->tx, entre_2->ty, 0);
				p->x = mapx * TILEX;		// coordonées entrée
				p->y = mapy * TILEY;
			}

			if (MAPDECOR[mapy][mapx] == 3)
				al_draw_scaled_bitmap(sortie_3->img, 0, 0, sortie_3->w, sortie_3->h, mapx * TILEX, mapy * TILEY, sortie_3->tx, sortie_3->ty, 0);

			if (MAPDECOR[mapy][mapx] == 4)
				al_draw_scaled_bitmap(gateau_4->img, 0, 0, gateau_4->w, gateau_4->h, mapx * TILEX, mapy * TILEY, gateau_4->tx, gateau_4->ty, 0);

			if (MAPDECOR[mapy][mapx] == 5) //set mobPop location
			{		
				XspawnTonneaux = mapx * TILEX;
				YspawnTonneaux = mapy * TILEY;
				//portal_tonneau1->x = mapx * TILEX;
				//portal_tonneau1->y = mapy * TILEY;
				//printf("portal_tonneau1->x : %d\nportal_tonneau1->y : %d\n", portal_tonneau1->x, portal_tonneau1->y);
				*isTonneaux = true;
			}

			if (MAPDECOR[mapy][mapx] == 6) {
				XdepopTonneaux = mapx * TILEX;	//set mobDepop location
				YdepopTonneaux = mapy * TILEY;
				//portal_tonneau2->x = mapx * TILEX;
				//portal_tonneau2->y = mapy * TILEY;
				//printf("portal_tonneau2->x : %d\nportal_tonneau2->y : %d\n", portal_tonneau2->x, portal_tonneau2->y);
			}

			// set coordonées sprites

			/*if (MAPDECOR[mapy][mapx] == 20) {
				gun->x = mapx * TILEX;
				gun->y = (mapy * TILEY) - gun->ty;
			}*/

			for (int i = 0; i < nbPortalMAX; i++) 
			{
				if (MAPDECOR[mapy][mapx] == 10 + i) 
				{
					found_count[i]++;
					*nbPortals += 1;
					portals[i][found_count[i]]->x = mapx * TILEX;
					portals[i][found_count[i]]->y = (mapy * TILEY) - (portals[i][found_count[i]]->ty - TILEY);
					/*printf("portals[%d][%d] x : %d \nportals[%d][%d] y : %d\n", i, found_count[i], portals[i][found_count[i]]->x,			//debug
						i, found_count[i], portals[i][found_count[i]]->y);
					printf("------------------------------------\n");*/
					if (found_count[i] == 0) 
					{
						portals[i][1]->desx = portals[i][0]->x;
						portals[i][1]->desy = portals[i][0]->y;
						//printf("DESTINATION portals[%d][1] desx : %d \nDESTINATION portals[%d][1] desy : %d\n", i, portals[i][1]->desx, i, portals[i][1]->desy);
						//printf("------------------------------------\n");
					}

					if (found_count[i] == 1) 
					{
						portals[i][0]->desx = portals[i][1]->x;
						portals[i][0]->desy = portals[i][1]->y;
						//printf("DESTINATION portals[%d][0] desx : %d \nDESTINATION portals[%d][0] desy : %d\n", i, portals[i][0]->desx, i, portals[i][0]->desy);
						//printf("------------------------------------\n");
					}
				}
			}
			al_set_target_backbuffer(display);
		}
	*nbPortals /= 2;
	printf("nbportals : %d\n", *nbPortals);
	l->h = SCREENX;
	l->w = SCREENY;
	l->tx = SCREENX;
	l->ty = SCREENY;

	//Nettoyage total et integrale
	al_destroy_bitmap(tile_0);
	al_destroy_bitmap(glados_1->img);
	al_destroy_bitmap(entre_2->img);
	al_destroy_bitmap(sortie_3->img);
	al_destroy_bitmap(gateau_4->img);
	free(glados_1);
	free(entre_2);
	free(sortie_3);
	free(gateau_4);

	return l;
}
void nbr_tonneaux_levels(int curlevel) {
	switch (curlevel) {
	case 0:
		nbTonneauxMAX = 10;
		vitesse_spawn = 50;
		break;
	case 1:
		nbTonneauxMAX = 10;
		vitesse_spawn = 50;
		break;
	case 2:
		nbTonneauxMAX = 15;
		vitesse_spawn = 50;
		break;
	case 4:
		nbTonneauxMAX = 6;
		vitesse_spawn = 30;
		break;
	case 6:
		nbTonneauxMAX = 0;
		vitesse_spawn = 200;
		break;
	case 7:
		nbTonneauxMAX = 55;
		vitesse_spawn = 10;
		break;
	}
}

tonneau*initTonneau(void) {
	tonneau*mob;

	mob = (tonneau*)malloc(sizeof(tonneau));
	mob->img = al_load_bitmap("./assets/images/boule.png");
	mob->angle = 0.18;
	mob->centerx = al_get_bitmap_width(mob->img) / 2;
	mob->centery = al_get_bitmap_height(mob->img) / 2;
	mob->dx = XspawnTonneaux;
	mob->dy = YspawnTonneaux;
	mob->pat = 5;
	mob->move = mob->pat;

	return mob;
}
player*initPlayer(void) {
	player*p = (player*)malloc(sizeof(player));
	char buffer[60];
	p->curFrame = 0;

	//load sprite
	for (int i = 0; i <= 6; i++) {
		sprintf_s(buffer, 60, "./assets/sprites/char/frame-%d.png", i);
		p->frames[i] = al_load_bitmap(buffer);

		if (!p->frames[i])
			beurre("CA LOOOAD PAAAAAAA\n");
		al_set_target_bitmap(p->frames[i]);
		al_set_target_backbuffer(display);
	}

	p->h = al_get_bitmap_height(p->frames[0]);
	p->w = al_get_bitmap_width(p->frames[0]);
	p->x = 0; //valeur par default
	p->y = SCREENY - p->ty;	//valeur par default
	p->tx = 64;
	p->ty = 64;
	p->flags = 0;

	return p;
}
sprite*initSprite(char*path, int tx, int ty, int size, int flags) {
	sprite*s = (sprite*)malloc(sizeof(sprite));
	char buffer[60];
	s->curFrame = 0;

	for (int i = 1; i <= size; i++) {
		sprintf_s(buffer, 60, path, i);
		s->frames[i - 1] = al_load_bitmap(buffer);

		if (!s->frames[i - 1])
			beurre("CA LOOOAD PAAAAAAA (sprite)\n");

		al_set_target_bitmap(s->frames[i - 1]);
		al_set_target_backbuffer(display);
	}
	s->h = al_get_bitmap_height(s->frames[0]);
	s->w = al_get_bitmap_width(s->frames[0]);
	s->tx = tx;
	s->ty = ty;

	s->size = size;
	s->flags = flags;

	return s;
}
portal*initPortal(char*path, int tx, int ty, int size) {
	portal*port = (portal*)malloc(sizeof(portal));
	char buffer[60];
	port->curFrame = 0;

	for (int i = 1; i <= size; i++) {
		sprintf_s(buffer, 60, path, i);
		port->frames[i - 1] = al_load_bitmap(buffer);

		if (!port->frames[i - 1])
			beurre("CA LOOOAD PAAAAAAA (portal)\n");

		al_set_target_bitmap(port->frames[i - 1]);
		al_set_target_backbuffer(display);
	}
	port->x = -999;
	port->y = -999;

	port->h = al_get_bitmap_height(port->frames[0]);
	port->w = al_get_bitmap_width(port->frames[0]);
	port->tx = tx;
	port->ty = ty;
	port->desx = -999;
	port->desy = -999;

	port->size = size;

	return port;
}
image*initImage(char*path, int tx, int ty) {
	image*i = (image*)malloc(sizeof(image));
	i->img = al_load_bitmap(path);
	i->w = al_get_bitmap_width(i->img);
	i->h = al_get_bitmap_height(i->img);
	i->tx = tx;
	i->ty = ty;
	al_set_target_backbuffer(display);
	return i;
}

/*********************************************
Deplacement
**********************************************/
void movePlayer(player*p) {
	bool noInput = true;
	////////////////////////////////////////// deplacements && controle des bords

	///////////////// NoClip

	if (al_key_down(&key, ALLEGRO_KEY_LCTRL)) {
		if (al_key_down(&key, ALLEGRO_KEY_RIGHT)) {
			p->x += 4;
			p->flags = 0;
			noInput = false;
		}
		if (al_key_down(&key, ALLEGRO_KEY_LEFT)) {
			p->x -= 4;
			p->flags = 1;
			noInput = false;
		}
		if (al_key_down(&key, ALLEGRO_KEY_DOWN)) {
			p->y += 4;
			noInput = false;
		}
		if (al_key_down(&key, ALLEGRO_KEY_UP)) {
			p->y -= 4;
			noInput = false;
		}
	}

	//////////////// deplacements normaux


	if (!al_key_down(&key, ALLEGRO_KEY_LCTRL)) {
		if (al_key_down(&key, ALLEGRO_KEY_RIGHT) && p->x < SCREENX - p->tx && boule_touche >= 0 && boss_boule_touche != -1) {
			p->x += 4;
			p->flags = 0;
			noInput = false;
		}
		if (al_key_down(&key, ALLEGRO_KEY_LEFT) && p->x > 0 && boule_touche <= 0) {
			p->x -= 4;
			p->flags = 1;
			noInput = false;
		}
		if (al_key_down(&key, ALLEGRO_KEY_DOWN) && p->y < SCREENY - p->ty) {
			p->y += 4;
			noInput = false;
		}
		if (al_key_down(&key, ALLEGRO_KEY_UP) && saut == 0 && boss_boule_touche != -1)
			saut++;

		//////////// control des bords

		if (p->x > SCREENX - p->tx)
			p->x = (SCREENX - p->tx) + 4;
		if (p->x < 0)
			p->x = 1;
		if (p->y > SCREENY - p->ty)
			p->y = (SCREENY - p->ty) + 4;
		if (p->y < 0)
			p->y = 0;

		///////////////////////// SAUT
		if (tp_CD > 45)
			saut = 0;
		if (saut >= 1) {
			saut++;
			if (saut >= 1 && saut <= 20)
				p->y -= 6;
			if (saut > 20)
				p->y += 6;
		}
		if (collision(p->x + TILEX, p->y, 0) && collision(p->x + TILEX + 1, p->y, 0))
			saut = 20;
		if (collision(p->x + TILEX, p->y + p->ty, 0) && collision(p->x + TILEX + 1, p->y + p->ty, 0))
			saut = 0;

		////////////////////////// GRAVITE

		if (!saut) {
			int compteur_gravite = 0;
			for (int i = 0; i < (p->tx / TILEX); i++)
				if (!collision(p->x + TILEX * i, p->y + p->ty, 0) && !collision(p->x + TILEX * i + 1, p->y + p->ty, 0))
					compteur_gravite++;

			if (compteur_gravite >= 4)
				p->y += 6;
		}

		//////////////////////////////////// COLLISIONS TILES

		for (int i = 0; i < (p->tx / TILEX); i++)
			if (collision(p->x + TILEX * i, p->y, 0) && collision(p->x + TILEX * i + 1, p->y, 0))

				p->y = ((p->y / TILEY)*TILEY) + TILEY;
		for (int i = 0; i < (p->tx / TILEX); i++)
			if (collision(p->x + TILEX * i, p->y + p->ty, 0) && collision(p->x + TILEX * i + 1, p->y + p->ty, 0))
				p->y = ((p->y / TILEY)*TILEY);

		for (int i = 0; i < (p->ty / TILEY); i++)
			if (collision(p->x, p->y + TILEX * i, 0) && collision(p->x, p->y + TILEX * i + 1, 0))
				p->x = ((p->x / TILEX)*TILEX) + TILEX;

		for (int i = 0; i < (p->ty / TILEY); i++)
			if (collision(p->x + p->tx, p->y + TILEY * i, 0) && collision(p->x + p->tx, p->y + TILEY * i + 1, 0))
				p->x = (p->x / TILEX)*TILEX;

	}		////////////////////////////// FIN


	//////////////////////////////// animation sprite joueur

	if (al_get_timer_count(timer) % 5 == 0) {
		if (!noInput)
			p->curFrame = (p->curFrame + 1) % 6;
		if (noInput && p->curFrame != 0)
			p->curFrame--;
	}

}
void moveTonneau(tonneau*mob) {

	if (!collision(mob->dx + TILEX, mob->dy, 0) && !collision(mob->dx + TILEX + 1, mob->dy, 0))		//gravitée
		mob->dy += 4;

	mob->dx += mob->move;	// incrementation x chaque tour

	if (mob->dx > SCREENX - mob->centerx)		//changement direction
		mob->move = -mob->pat;
	if (mob->dx < 0 + mob->centerx)		//haut / bas
		mob->move = mob->pat;

	if (collision(mob->dx - 8, mob->dy, 0) && collision(mob->dx - 8, mob->dy -16, 0))		//les coter	
		mob->move = mob->pat;
	if (collision(mob->dx + 8, mob->dy, 0) && collision(mob->dx + 8, mob->dy - 16, 0)){
		mob->move = -mob->pat;
	}


	if (mob->move == mob->pat)		//rotation en fonction de la direction
		mob->angle += 0.18;
	if (mob->move == -mob->pat)
		mob->angle -= 0.18;

	if (mob->dx >= XdepopTonneaux && mob->dx <= XdepopTonneaux + 32 &&
		mob->dy >= YdepopTonneaux && mob->dy <= YdepopTonneaux + 32) {

		mob->dx = XspawnTonneaux;
		mob->dy = YspawnTonneaux;
	}
	if (curLevel != 7)
		while (collision(mob->dx + TILEX, mob->dy, 0))
			mob->dy -= 1;
}

int saut_menu(sprite*pl, int saut_regles) {
	saut_regles++;
	if (saut_regles >= 38)
		saut_regles = 0;

	if (saut_regles >= 1) {

		if (saut_regles >= 1 && saut_regles < 20)
			pl->y -= 5;
		if (saut_regles >= 20)
			pl->y += 5;
	}
	return saut_regles;
}

void addTonneau(int nb, int delay) {
	if (nbTonneaux <= nbTonneauxMAX)
		if (al_get_timer_count(timer) % delay == 0)
			nbTonneaux += nb;
}
void animSprite(sprite*s) {
	s->curFrame = (s->curFrame + 1) % s->size;
}
void animPortal(portal*port) {
	port->curFrame = (port->curFrame + 1) % port->size;
}
/*********************************************
Collisions
*********************************************/
bool collision(int x, int y, int nbTile) {
	if (MAPDECOR[y / TILEY][x / TILEX] == nbTile)
		return true;
	return false;
}
void PortalsCollision(player*p, portal*portals[][2], int*nbPortals) {
	if (tp_CD > 0)
		tp_CD--;
	al_get_keyboard_state(&key);
	if (tp_CD == 0 && al_key_down(&key, ALLEGRO_KEY_UP) || tp_CD == 0 && al_key_down(&key, ALLEGRO_KEY_DOWN)) {
		bool found = false;
		int tempPX;
		int tempPY;
		for (int po = 0; po < nbPortals; po++) {

			for (int i = 0; i < (p->tx / TILEX); i++)
				if (collision(p->x + TILEX * i, p->y, 10 + po) && collision(p->x + TILEX * i + 1, p->y, 10 + po)) {
					found = true;
					tempPX = p->x + TILEX * i;
					tempPY = p->y;
				}

			for (int i = 0; i < (p->tx / TILEX); i++)
				if (collision(p->x + TILEX * i, p->y + p->ty, 10 + po) && collision(p->x + TILEX * i + 1, p->y + p->ty, 10 + po)) {
					found = true;
					tempPX = p->x + TILEX * i;
					tempPY = p->y + p->ty;
				}

			for (int i = 0; i < (p->ty / TILEY); i++)
				if (collision(p->x, p->y + TILEX * i, 10 + po) && collision(p->x, p->y + TILEX * i + 1, 10 + po) ||
					collision(p->x - (portals[0][0]->tx - TILEX), p->y + TILEX * i, 10 + po) &&
					collision(p->x - (portals[0][0]->tx - TILEX), p->y + TILEX * i + 1, 10 + po)) {
					found = true;
					tempPX = p->x;
					tempPY = p->y + TILEX * i;
				}

			for (int i = 0; i < (p->ty / TILEY); i++)
				if (collision(p->x + p->tx, p->y + TILEY * i, 10 + po) && collision(p->x + p->tx, p->y + TILEY * i + 1, 10 + po) ||
					collision((p->x + p->tx) - (portals[0][0]->tx - TILEX), (p->y + TILEY * i) - (portals[0][0]->tx - TILEX), 10 + po) &&
					collision((p->x + p->tx) - (portals[0][0]->tx - TILEX), (p->y + TILEY * i + 1) - (portals[0][0]->tx - TILEX), 10 + po)) {
					found = true;
					tempPX = p->x + p->tx;
					tempPY = p->y + TILEY * i;
				}
		}


		if (found) {
			for (int k = 0; k < nbPortals; k++)
				for (int l = 0; l < 2; l++)
					if (tempPX / TILEX >= portals[k][l]->x / TILEX &&
						tempPX / TILEX <= portals[k][l]->x / TILEX + portals[k][l]->tx / TILEX
						&&
						tempPY / TILEY >= portals[k][l]->y / TILEY &&
						tempPY / TILEY <= portals[k][l]->y / TILEY + portals[k][l]->ty / TILEY) {
						//printf("portals[%d][%d]->x : %d || p->x : %d\n", k, l, portals[k][l]->x / TILEX, tempPX / TILEX);
						//printf("portals[%d][%d]->y : %d || p->y : %d\n", k, l, portals[k][l]->y / TILEY, tempPY / TILEY);
						p->x = portals[k][l]->desx;
						p->y = portals[k][l]->desy;
						tp_CD = 50;
					}
		}
	}
}
void collisionJM(player*p, tonneau*mob[]) {
	for (int i = 0; i < nbTonneaux; i++) {
		if (mob[i]->move < 0) {
			if (mob[i]->dy >= p->y && mob[i]->dy <= p->y + 64 && p->x >= mob[i]->dx - mob[i]->centerx * 4 && p->x <= mob[i]->dx - mob[i]->centerx) {

				p->x += mob[i]->move;
				if (mob[i]->move > 0)
					boule_touche = 1;

				else
					boule_touche = -1;
			}
			else
				boule_touche = 0;
		}
		if (mob[i]->move > 0) {
			if (mob[i]->dy >= p->y && mob[i]->dy <= p->y + 64 && p->x >= mob[i]->dx - mob[i]->centerx && p->x <= mob[i]->dx + mob[i]->centerx) {

				p->x += mob[i]->move;
				if (mob[i]->move > 0)
					boule_touche = 1;
				else
					boule_touche = -1;
			}
			else
				boule_touche = 0;
		}

		if (boule_touche != 0)
			break;
	}
}
/*********************************************
Affichage
**********************************************/
void affichePlayer(player*p) {
	al_draw_scaled_bitmap(p->frames[p->curFrame], 0, 0, p->w, p->h, p->x, p->y, p->tx, p->ty, p->flags);
}
void afficheSprite(sprite*s) {
	al_draw_scaled_bitmap(s->frames[s->curFrame], 0, 0, s->w, s->h, s->x, s->y, s->tx, s->ty, s->flags);
}
void affichePortal(portal*port) {
	al_draw_scaled_bitmap(port->frames[port->curFrame], 0, 0, port->w, port->h, port->x, port->y, port->tx, port->ty, 0);
}
void afficheImage(image*i, int x, int y, int flags) {
	al_draw_scaled_bitmap(i->img, 0, 0, i->w, i->h, x, y, i->tx, i->ty, flags);
}
void afficheTonneau(tonneau*mob) {
	al_draw_rotated_bitmap(mob->img, mob->centerx, mob->centery, mob->dx, mob->dy - mob->centerx, mob->angle, 0);
}

void aficheTimer(const char*txt,int m, int s, int ms, int x, int y, ALLEGRO_COLOR color) {
	char buffer[80];
	sprintf_s(buffer, 80, txt ,m, s, ms);
	al_draw_text(font_2, color, x, y, ALLEGRO_ALIGN_CENTRE, buffer);
}
//debug
void drawMatrix() {
	ALLEGRO_BITMAP*img = al_create_bitmap(2, 2);
	al_set_target_bitmap(img);
	al_clear_to_color(al_map_rgb(255, 0, 0));
	al_set_target_backbuffer(display);
	for (int i = 0; i < MAPTX; i++)
		for (int j = 0; j < MAPTY; j++)
			al_draw_bitmap(img, i*TILEX, j*TILEY, 0);
}
void drawPortalsHitBox(portal*portals[][2]) {
	ALLEGRO_BITMAP*img = al_create_bitmap(48, 80);
	al_set_target_bitmap(img);
	al_clear_to_color(al_map_rgb(255, 0, 0));
	al_set_target_backbuffer(display);
	for (int i = 0; i < nbPortalMAX; i++)
		for (int j = 0; j < 2; j++)
			al_draw_bitmap(img, portals[i][j]->x, portals[i][j]->y, 0);
}
void drawBoulesHitBox(tonneau*mob[]) {
	ALLEGRO_BITMAP*img = al_create_bitmap(16, 2);
	al_set_target_bitmap(img);
	al_clear_to_color(al_map_rgb(255, 0, 0));
	al_set_target_backbuffer(display);
	for (int i = 0; i < nbTonneaux; i++) {

		al_draw_bitmap(img, mob[i]->dx, mob[i]->dy, 0);
	}
}
/***********************************************
SPECIAL BOSS
************************************************/
void add_bossTonneau(int nb, int delay) {
	if (nbBossTonneaux <= nbTonneauxMAX)
		if (al_get_timer_count(timer) % delay == 0)
			nbBossTonneaux += nb;
}
void move_bossTonneau(tonneau*mob) {

	mob->dx -= mob->move;	// incrementation x chaque tour

	mob->angle -= 0.18;

	if (mob->dx <= 16) {
		mob->dx = mob->spawnX;
		mob->dy = mob->spawnY;
	}

}
void collisionJM_bossTonneau(player*p, tonneau*mob[9][nbBossMobMAX]) {
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < nbBossTonneaux; j++) {
			if (mob[i][j]->dy >= p->y && mob[i][j]->dy <= p->y + 64 && p->x >= mob[i][j]->dx - mob[i][j]->centerx * 4 && p->x <= mob[i][j]->dx - mob[i][j]->centerx) {
				p->x -= mob[i][j]->move;
				boss_boule_touche = -1;
			}
			else
				boss_boule_touche = 0;

			/*if (boss_boule_touche != 0)
				break;*/
		}
}
void affiche_textGlados(int curTextGlados) {
	al_draw_text(font_3, al_map_rgb(233, 0, 255), SCREENX - (TILEX * 22), SCREENY / 2 + TILEY, ALLEGRO_ALIGN_CENTRE, textGaldos[curTextGlados]);
}
void affiche_textDebut()
{
	al_draw_text(font_3, al_map_rgb(233, 0, 255), SCREENX / 1.5, SCREENY / 2, ALLEGRO_ALIGN_CENTRE, "Cette situation ne te rappelle pas un merveilleux passe ?");
	al_draw_text(font_3, al_map_rgb(233, 0, 255), SCREENX / 1.5, SCREENY / 1.9, ALLEGRO_ALIGN_CENTRE, "Celui ou tu etais mon cobaye ... Mais desormais les temps sont revolues.");
	al_draw_text(font_3, al_map_rgb(233, 0, 255), SCREENX / 1.5, SCREENY / 1.8, ALLEGRO_ALIGN_CENTRE, "Cette epreuve se deroulera sous plusieurs cartes jusqu'a celle finale ");
	al_draw_text(font_3, al_map_rgb(233, 0, 255), SCREENX / 1.5, SCREENY / 1.7, ALLEGRO_ALIGN_CENTRE, "qui en sera un peu plus trompeuses et ce sera celle qui te sanctionnera le plus !");
	al_draw_text(font_3, al_map_rgb(233, 0, 255), SCREENX / 1.5, SCREENY / 1.63, ALLEGRO_ALIGN_CENTRE, "Comme promis, tu n'as aucune crainte a te faire.");
	al_draw_text(font_3, al_map_rgb(233, 0, 255), SCREENX / 1.5, SCREENY / 1.54, ALLEGRO_ALIGN_CENTRE, "J'ai construis cette epreuve de maniere a ce qu'une mort ne soit pas realisable !");
	al_draw_text(font_3, al_map_rgb(233, 0, 255), SCREENX / 1.5, SCREENY / 1.46, ALLEGRO_ALIGN_CENTRE, "Je te souhaite bon courage ma cherie");
}
/*********************************************
					TOOLS
**********************************************/
void copyTableau(int*tab1[][MAPTX], int*tab2[][MAPTX]) {
	for (int mapy = 0; mapy < MAPTY; mapy++)
		for (int mapx = 0; mapx < MAPTX; mapx++)
			tab1[mapy][mapx] = tab2[mapy][mapx];
}
void beurre(const char*txt) {
	ALLEGRO_DISPLAY*d;
	d = al_is_system_installed() ? al_get_current_display() : NULL;
	al_show_native_message_box(d, "Beurre", txt, NULL, NULL, 0);
	exit(EXIT_FAILURE);
}
int is_key_pressed(ALLEGRO_KEYBOARD_STATE*key, int touche, int repeat) {
	//le tableau conserve ses valeurs d'un appel � l'autre (static)
	static int press[ALLEGRO_KEY_MAX] = { 0 };
	int res = 0;

	if (al_key_down(key, touche) && press[touche] < repeat) {
		press[touche]++;
		res = 1;
	}
	else if (!al_key_down(key, touche))
		press[touche] = 0;
	return res;
}
/*****************************************************************
*****************************************************************/