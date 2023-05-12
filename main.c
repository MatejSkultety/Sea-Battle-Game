#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DEFAULT '~'
#define PLACED_SHIP 'O'
#define HIT '0'
#define MISS '*'
#define SUNK 'X'

#define INVALID 0
#define VALID_MISS 1
#define VALID_HIT 2

#define DEFAULT_COLOR "\033[0m"
#define RED_COLOR "\033[31m"
#define GREEN_COLOR "\033[32m"
#define BLUE_COLOR "\033[36m"
#define YELLOW_COLOR "\033[33m"
#define BRIGHT_RED_COLOR "\033[91m"
#define BRIGHT_GREEN_COLOR "\033[92m"
#define UNDERLINE_COLOR "\033[4;37m"

#define HORIZONTAL 1
#define VERTICAL 0

#define MAX_NAME 32
#define NULL_COORD {27, 27}     // bigger than max board-size => never used

#ifdef _WIN32
    #define CONSOLE system("cls")   // makes ANSI work
#else
    #define CONSOLE 1			// literary makes nothing
#endif

typedef struct player_fleet {
    char nick[MAX_NAME];
    char* carrier[5];
    char* battleship[4];
    char* destroyer[3];
    char* submarine[3];
    char* patrol_boat[2];
    char** player_board;
    char* last_shot;
} PLAYER;

typedef struct ship {
    unsigned short orientation;
    unsigned short size;
    unsigned short x;
    unsigned short y;
} SHIP;

typedef struct coord {
    unsigned short x;
    unsigned short y;
} _COORD;

//////////////// CONSOLE GRAPHICS /////////////////

void clear_screen();
void print_image();

/////////////// PRINTING TO CONSOLE ///////////////

void print_one(PLAYER player, unsigned int board_size);
void print_both(PLAYER player_active, PLAYER player_opponent, unsigned int board_size);
void print_tables(PLAYER player_active, PLAYER player_opponent, unsigned int board_size);
void alive_ship_check(char* pShip[]);
void default_screen(PLAYER player_active, PLAYER player_opponent, unsigned int board_size);
void print_hint();

//////////////// UI, MENU and GUIDE ///////////////

void main_menu();
void main_menu_intro();
void main_menu_help();

/////////////////// INITIALIZING //////////////////

char** initialize(char** board, unsigned int board_size);
void free_board(char** board, unsigned int board_size);
PLAYER placement_of_ships_user(unsigned int board_size);
PLAYER placement_of_ships_computer(unsigned int board_size);
int place_ship(char** board, unsigned int board_size, SHIP active_ship);
void set_fleet(SHIP active_ship, unsigned short ship_size, int flag, PLAYER *result);

///////////////////// GAMEPLAY ////////////////////

void player_vs_player(unsigned int board_size);
void player_vs_computer(unsigned int board_size);
int player_turn(PLAYER* player_active, PLAYER* player_opponent, unsigned int board_size);
_COORD get_coord();
int fire(PLAYER player_opponent, _COORD aim, unsigned int board_size);
int ship_hit_check(PLAYER player_opponent);
int victory_check(PLAYER player_opponent);

//////////////////////// AI ///////////////////////

int computer_turn(PLAYER* player_active, PLAYER* player_opponent, unsigned int board_size, _COORD *focused_target);
_COORD calculate_shot(PLAYER* player_active, PLAYER* player_opponent, unsigned int board_size, _COORD *focused_target);
int line_fire_right(PLAYER* player_opponent, unsigned int board_size, _COORD* calculation);
int line_fire_left(PLAYER* player_opponent, unsigned int board_size, _COORD* calculation);
int line_fire_up(PLAYER* player_opponent, unsigned int board_size, _COORD* calculation);
int line_fire_down(PLAYER* player_opponent, unsigned int board_size, _COORD* calculation);
int calculation_check(PLAYER* player_opponent, unsigned int board_size, short x, short y);
_COORD find_last_hit(PLAYER* player_opponent, unsigned int board_size);
_COORD random_shot(unsigned int board_size);


int main() {
    CONSOLE;    // makes ANSI work on Win CMD
    printf(DEFAULT_COLOR);
    char user_input[3];
    do {
        main_menu();    // game starts

        printf("\n\tType 'R' for Restart or anything else to leave: ");
        fgets(user_input, 3, stdin);
        user_input[0] = tolower(user_input[0]);
        clear_screen();
    } while(user_input[0] == 'r');  // loop until user types R for RESTART

    return 0;
}


/////////////////////////////////////////////////////
////////////////// CONSOLE GRAPHICS /////////////////
/////////////////////////////////////////////////////


void clear_screen() {
    /* Clears the screen and sets color to default */

    printf("\033c"DEFAULT_COLOR);
}


void print_image() {
    /* Prints beautiful picture of USS Arizona (it actually works) */

    printf("                  \033[4;33m** \033[0;37m=====00       #############      |\033[4;31mKNM\033[0;37m      ");
    printf("\n\t      ###########        ####    # O  O  O  O  O #    |  ####   ");
    printf("\n\t\033[4;34m ~ ~ ~ ~ ~ ~ ~\033[0;37m #####=============#===============#########\033[4;34m~ ~ ~ ~ ~");
    printf("\n\t\033[4;34m~ ~ ~ ~ ~ ~ ~ ~ ~\033[0;37m###################################\033[4;34m~ ~ ~ ~ ~ ~ ~ ~");
    printf("\n\t\033[4;34m ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~\n\n");
}


///////////////////////////////////////////////////
/////////////// PRINTING TO CONSOLE ///////////////
///////////////////////////////////////////////////


void print_one(PLAYER player, unsigned int board_size) {
    /* Prints the game-board of given player while placing fleet */

    // first row
    printf(DEFAULT_COLOR"\n\t   ");
    for (int i = 0; i < board_size; ++i) {
        printf("%2d ", i + 1);  // 0 1 2 ...
    }
    printf("\n");

    // rows with tiles
    for (int i = 0; i < board_size; i++) {
        printf(DEFAULT_COLOR"\n\t%c  ", 'A' + i);
        for (int j = 0; j < board_size; j++) {
            if (player.player_board[j][i] == DEFAULT) printf(BLUE_COLOR);     // sets color
            else printf(GREEN_COLOR);

            printf(" %c ", player.player_board[j][i]);
        }
    }
    printf("\n\n"DEFAULT_COLOR);
}


void print_both(PLAYER player_active, PLAYER player_opponent, unsigned int board_size) {
    /* Prints the game-board of given player on left and prints
    other board, where one can see his shots and hits on right */

    // header
    printf(DEFAULT_COLOR"\n\tYour fleet:");
    for (int i = 0; i < board_size; ++i) printf("   ");
    printf("\t\tHits and Misses:\n");

    // first row
    printf("\n\t   ");
    for (int i = 0; i < board_size; ++i) printf("%2d ", i + 1);  // left board
    printf("\t\t\t   ");

    for (int i = 0; i < board_size; ++i) printf("%2d ", i + 1); // right board
    printf("\n");

    // rows with tiles
    for (int i = 0; i < board_size; i++) {  // for each line:
        printf(DEFAULT_COLOR"\n\t%c  ", 'A' + i);
        for (int j = 0; j < board_size; j++) {    // left board
            if (player_active.player_board[j][i] == DEFAULT) printf(BLUE_COLOR);     // sets color according to tile
            else if (player_active.player_board[j][i] == PLACED_SHIP) printf(GREEN_COLOR);
            else if (player_active.player_board[j][i] == HIT) printf(BRIGHT_RED_COLOR);
            else if (player_active.player_board[j][i] == MISS) printf(YELLOW_COLOR);
            else if (player_active.player_board[j][i] == SUNK) printf(RED_COLOR);

            printf(" %c ", player_active.player_board[j][i]);   // prints sign
        }

        printf("\t\t");
        printf(DEFAULT_COLOR"\t%c  ", 'A' + i);
        for (int j = 0; j < board_size; j++) {    // right board
            if (player_opponent.player_board[j][i] == DEFAULT || player_opponent.player_board[j][i] == PLACED_SHIP) {
                printf(BLUE_COLOR);
                printf(" ~ ");          // cannot tell, if enemy ship's there
                continue;
            }
            else if (player_opponent.player_board[j][i] == HIT) printf(BRIGHT_RED_COLOR);
            else if (player_opponent.player_board[j][i] == MISS) printf(YELLOW_COLOR);
            else if (player_opponent.player_board[j][i] == SUNK) printf(RED_COLOR);

            printf(" %c ", player_opponent.player_board[j][i]);
        }
    }
    printf(DEFAULT_COLOR"\n\n");
}


void print_tables(PLAYER player_active, PLAYER player_opponent, unsigned int board_size) {
    /* Prints two tables of 5 ships. Active ships are printed green and sunk are red */

    // header
    printf(BLUE_COLOR"\t   %s's", player_active.nick);
    printf(DEFAULT_COLOR" fleet:");

    for (int i = 0; i < board_size*3 - strlen(player_active.nick) + 19; ++i) printf(" ");   // spaces

    printf(BRIGHT_RED_COLOR"%s's", player_opponent.nick);
    printf(DEFAULT_COLOR" fleet:\n");

    // table
    // Carrier 5
    alive_ship_check(player_active.carrier);
    printf("\t   Carrier [5]");     // left table (active)
    for (int i = 0; i < board_size + 4; ++i) printf("   ");     // spaces
    alive_ship_check(player_opponent.carrier);
    printf("     Carrier [5]\n");   // right table (opponent)
    // Battleship 4
    alive_ship_check(player_active.battleship);
    printf("\t   Battleship [4]");
    for (int i = 0; i < board_size + 4; ++i) printf("   ");
    alive_ship_check(player_opponent.battleship);
    printf("  Battleship [4]\n");
    // Destroyer 3
    alive_ship_check(player_active.destroyer);
    printf("\t   Destroyer [3]");
    for (int i = 0; i < board_size + 4; ++i) printf("   ");
    alive_ship_check(player_opponent.destroyer);
    printf("   Destroyer [3]\n");
    // Submarine 3
    alive_ship_check(player_active.submarine);
    printf("\t   Submarine [3]");
    for (int i = 0; i < board_size + 4; ++i) printf("   ");
    alive_ship_check(player_opponent.submarine);
    printf("   Submarine [3]\n");
    // Patrol Boat 2
    alive_ship_check(player_active.patrol_boat);
    printf("\t   Patrol Boat [2]");
    for (int i = 0; i < board_size + 4; ++i) printf("   ");
    alive_ship_check(player_opponent.patrol_boat);
    printf(" Patrol Boat [2]\n");
}


void alive_ship_check(char* pShip[]) {
    /* Checks, whether tiles of given ship are set to SUNK. If the ship is sunk,
    color is set to red, otherwise the color is set to green */

    if (*pShip[0] == SUNK) printf(RED_COLOR);
    else printf(GREEN_COLOR);
}


void default_screen(PLAYER player_active, PLAYER player_opponent, unsigned int board_size) {
    /* Prints name of player who is on turn, sums up last round, prints his and opponent's boards and
    tables with active ships of both players */

    clear_screen();
    printf(BLUE_COLOR"\n\t%s's", player_active.nick);
    printf(DEFAULT_COLOR" turn\n");

    printf("\n\tSTATUS: ");     // dialogue
    if (player_active.last_shot) {  // based on last hit
        switch (*player_active.last_shot) {
            case MISS:
                printf(YELLOW_COLOR"'We missed Sir!' ");
                break;
            case HIT:
                printf(BRIGHT_GREEN_COLOR"'Great hit %s!' ", player_active.nick);
                break;
            case SUNK:
                printf(GREEN_COLOR"'Good jop %s! Enemy vessel is sinking.' ", player_active.nick);
                break;
        }
    } else printf("'Reporting for duty %s! What are your commands?' ", player_active.nick);    // first round

    if (player_opponent.last_shot) {
        switch (*player_opponent.last_shot) { // based on last hit
            case MISS:
                printf(YELLOW_COLOR"'Enemy shell missed us.'\n");
                break;
            case HIT:
                printf(BRIGHT_RED_COLOR"'We've been hit!'\n");
                break;
            case SUNK:
                printf(RED_COLOR"'Our boat is sinking! Mayday!'\n");
                break;
        }
    } else printf("'Let's hunt these dogs.'\n"DEFAULT_COLOR);

    print_both(player_active, player_opponent, board_size);
    print_tables(player_active, player_opponent, board_size);
    print_hint();
}


void print_hint() {
    /* Prints one line with description of different chars shown on board */

    printf(BLUE_COLOR"\n\t%c", DEFAULT);
    printf(DEFAULT_COLOR" - undiscovered water |");

    printf(GREEN_COLOR"  %c", PLACED_SHIP);
    printf(DEFAULT_COLOR" - your ship |");

    printf(YELLOW_COLOR"  %c", MISS);
    printf(DEFAULT_COLOR" - miss (empty tile) |");

    printf(BRIGHT_RED_COLOR"  %c", HIT);
    printf(DEFAULT_COLOR" - ship is hit |");

    printf(RED_COLOR"  %c", SUNK);
    printf(DEFAULT_COLOR" - sunk ship");
}


///////////////////////////////////////////////////
//////////////// UI, MENU and GUIDE ///////////////
///////////////////////////////////////////////////


void main_menu() {
    /* Main menu where user gives all important information
    and gets needed guidance */

    clear_screen();
    char user_input[8];

    main_menu_intro();
    fgets(user_input, 8, stdin);    // chance to get help
    if (!strcmp(user_input, "h\n") || !strcmp(user_input, "H\n")) main_menu_help();

    printf(UNDERLINE_COLOR"\n\tType size");
    printf(DEFAULT_COLOR" of the board in range 5 to 26 (e.g. '10'): ");

    fgets(user_input, 8, stdin);    // inputs board size
    unsigned int board_size = strtol(user_input, NULL, 10);
    if (board_size > 26) board_size = 26;   // max size
    if (board_size < 5) board_size = 5;   // min size

    printf(UNDERLINE_COLOR"\n\n\tChoose game-mode:");
    printf(DEFAULT_COLOR"\n\tType '1' for: Player vs Player;");
    printf("\n\tType '2' for: Player vs Computer;");
    printf("\n\tType anything else to leave: ");

    fgets(user_input, 8, stdin);    // inputs game-mode
    clear_screen();
    switch (user_input[0]) {
        case '1':
            player_vs_player(board_size);
            break;
        case '2':
            player_vs_computer(board_size);
            break;
        default:
            break;
    }
}


void main_menu_intro() {
    /* Only prints title and basic instructions to console */

    printf(BLUE_COLOR"\n\tSEA ");
    printf(RED_COLOR"BATTLE");
    printf("\n\t------------\n\t");
    print_image();
    printf(DEFAULT_COLOR"\n\tWelcome to Main Menu!");
    printf("\n\tSea Battle is fascinating turn-based strategy game for two players (recommended to play");
    printf("\n\tagainst AI). You can choose to play against computer or friend. Each player has fleet with");
    printf("\n\t5 ships of different length. Goal of the game is to sink all of the enemy ships.");
    printf("\n\tFirstly, you place all of your ships to board. When the game starts, you have to");
    printf("\n\ttype coordinates of enemy board (you'll shoot there). Your shots will be tracked");
    printf("\n\ton right side of the screen. Underneath you'll see active ships of both players.");
    printf("\n\tThere is STATUS bar on top of the screen, which sums up recent round. You can choose");
    printf("\n\tsize of the game-board. Beware that small board results into fast clash and large");
    printf("\n\tprolongs the battle (10 is recommended - with higher go FULLSCREEN). Good luck!\n");
    printf("\n\tType 'H' for additional hint or type anything else to proceed: ");
}


void main_menu_help() {
    /* Only prints hint to console */

    printf("\n\tFleet consists of Carrier (5), Battleship (4), Destroyer (3), Submarine (3) and Patrol Boat (2).");
    printf("\n\tBefore game, you'll place your ships anywhere vertically or horizontally to board of given size.");
    printf("\n\tDuring battle, you'll alternate turns with your opponent. If you play against human, closing eyes");
    printf("\n\twhile opponent is on turn is recommended. When you are on turn, your name will be displayed on top");
    printf("\n\tof the screen. STATUS will tell you, whether your last shot hit the enemy and whether we were hit.");
    printf("\n\tTwo boards will be displayed. Left shows your ships and enemy strikes. Right tracks your shots.");
    printf("\n\tUnder each board lies table with player's ships (red - sunk, green - active). Each turn you have to");
    printf("\n\ttype coordinates of one enemy tile (e.g.A2). Goal is to guess position of opponents ships and hit it.");
    printf("\n\tWhen player doesn't have any active ships, his opponent wins and victory screen pops up. GL-HF!\n");
}


///////////////////////////////////////////////////
/////////////////// INITIALIZING //////////////////
///////////////////////////////////////////////////


char** initialize(char** board, unsigned int board_size) {
    /* Allocates memory for game board and sets value of
    each tile to DEFAULT */

    board = (char**) malloc(sizeof(char*) * board_size);
    for (int i = 0; i < board_size; ++i) {
        board[i] = (char*) malloc(sizeof(char) * board_size);

        for (int j = 0; j < board_size; j++) board[i][j] = DEFAULT;
    }
    return board;
}


void free_board(char** board, unsigned int board_size) {
    /* Frees allocated memory for game board */

    for (int i = 0; i < board_size; ++i) free(board[i]);
    free(board);
}


PLAYER placement_of_ships_user(unsigned int board_size) {
    /* Memory for board will be allocated. Fleet of each player consists of five vessels.
    Player enters his nick, enters coordinates and places his ships to the board */

    clear_screen();
    PLAYER result;
    printf("\n\tEnter your nick: ");
    fgets(result.nick, MAX_NAME, stdin);
    char* new_line = strchr(result.nick, '\n'); // removes '\n'
    if(new_line) *new_line = '\0';

    char** player_board = NULL;  // allocates memory
    player_board = initialize(player_board, board_size);
    result.player_board = player_board;
    result.last_shot = NULL;    // no last shot yet

    char buffer[8];
    int ship_size = 5;
    int flag = 1;   // flag 1 = destroyer; flag 0 = submarine (both size 3)
    SHIP active_ship;

    while (ship_size >= 2) {    // this WHILE will run 5 times

        while(1) {  // this WHILE will run until active ship is placed (in case of formatting errors
            printf("\n\t%s is now placing his ships!\n", result.nick);
            print_one(result, board_size);

            printf(UNDERLINE_COLOR);
            switch (ship_size) {
                case 5:
                    printf("\n\tPlacing Carrier (5)\n");
                    break;
                case 4:
                    printf("\n\tPlacing Battleship (4)\n");
                    break;
                case 3:
                    if (flag) printf("\n\tPlacing Destroyer (3)\n");
                    else printf("\n\tPlacing Submarine (3)\n");
                    break;
                default:
                    printf("\n\tPlacing Patrol Boat (2)\n");
                    break;
            }
            printf(DEFAULT_COLOR"\n\tType coordinates of ");
            printf(UNDERLINE_COLOR"TOP or LEFT");
            printf(DEFAULT_COLOR" corner of the active ship\n\tfollowed with ");
            printf(UNDERLINE_COLOR"'H' for horizontal or 'V' for vertical");
            printf(DEFAULT_COLOR" orientation.\n\tIf the place is occupied, you will be asked again.");
            printf("\n\tEXAMPLE: placing Destroyer (3) at 'B2 H' places the ship to tiles [B2][B3][B4]");
            printf("\n\n\tEnter ship placement (e.g. 'A2 H'): ");

            fgets(buffer, 8, stdin);
            active_ship.x = strtol(buffer + 1, NULL, 10) - 1;   // reading x coordinate
            buffer[0] = toupper(buffer[0]);
            active_ship.y = buffer[0] - 'A';    // reading y coordinate

            char *space = strchr(buffer, ' ');
            if(space) { // if not NULL due to bad formatting
                space++;
                *space = toupper(*space);
                if (*space == 'V') active_ship.orientation = VERTICAL;      // reading orientation
                else active_ship.orientation = HORIZONTAL;

                active_ship.size = ship_size;
                clear_screen();
            }
            else {  // space == NULL -> formatting Error
                clear_screen();
                printf("\n\tFormatting ERROR\n");
                continue;   // repeat
            }

            if (place_ship(player_board, board_size, active_ship)) {     // checks if there is an obstacle
                printf(RED_COLOR"\n\tUnable to place here, restarting...\n"DEFAULT_COLOR);
            }
            else {      // ship has been placed
                result.player_board = player_board; // saves created placement
                set_fleet(active_ship, ship_size, flag, &result);
                break;  // ship is placed into board and into player struct - breaks inner WHILE
            }
        }

        if (ship_size == 3 && flag) { // taking care of two ships with size 3
            ship_size++;
            flag = 0;
        }
        ship_size--;    // placing ships from largest to smallest
    }

    printf("\n\t%s is now placing his ships!\n", result.nick);  // shows the result
    print_one(result, board_size);
    printf("\n\tType 'R' for RESTART if you are unsatisfied and want to start over.");
    printf("\n\tType anything else to confirm and continue: ");
    fgets(buffer, 8, stdin);
    buffer[0] = tolower(buffer[0]);

    if (buffer[0] == 'r') {
        free_board(result.player_board, board_size);
        return placement_of_ships_user(board_size); // recursion - repeating the process
    }
    else return result;     // user is satisfied - returns the result struct
}


PLAYER placement_of_ships_computer(unsigned int board_size) {
    /* Allocates memory for computer's board and tries to place ships. Generates random coordinates and
    if space is occupied, generates other etc. When 5 ships are placed, returns resulting struct */

    PLAYER result;
    strcpy(result.nick, "COMPUTER");

    char** player_board = NULL;  // allocates memory
    player_board = initialize(player_board, board_size);
    result.player_board = player_board;
    result.last_shot = NULL;

    int ship_size = 5;
    int flag = 1;   // flag 1 = destroyer; flag 0 = submarine (both size 3)
    SHIP active_ship;

    while (ship_size >= 2) {    // this while will run 5 times

        while(1) {  // this WHILE will run until active ship is placed

            active_ship.size = ship_size;
            active_ship.orientation = rand()%2; // 0 for VERTICAL, 1 for HORIZONTAL
            active_ship.x = rand()%board_size;
            active_ship.y = rand()%board_size;    // generates random ship position

            if (place_ship(player_board, board_size, active_ship)) continue;  // checks if there is an obstacle
            // ship has been placed
            result.player_board = player_board; // saves created placement
            set_fleet(active_ship, ship_size, flag, &result);
            break;  // ship is placed into board and into player struct - breaks WHILE
        }

        if (ship_size == 3 && flag) { // taking care of two ships with size 3
            ship_size++;
            flag = 0;
        }
        ship_size--;
    }
    return result;
}


int place_ship(char** player_board, unsigned int board_size, SHIP active_ship) {
    /* Checks, whether ship can be placed to active_ship coordinates. If yes, function
    places ship to the board and returns 0. If it is impossible to place ship there,
    function returns 1 and doesn't edit the board */

    if (active_ship.x < 0 || active_ship.x >= board_size) return 1;
    if (active_ship.y < 0 || active_ship.y >= board_size) return 1;

    if (active_ship.orientation == HORIZONTAL) {  // horizontal orientation
        if (active_ship.x + active_ship.size > board_size) return 1;    // doesn't fit to board

        for (int i = 0; i < active_ship.size; ++i) {
            if (player_board[active_ship.x + i][active_ship.y] != DEFAULT) return 1;   // tile is occupied
        }

        for (int i = 0; i < active_ship.size; ++i) {
            player_board[active_ship.x + i][active_ship.y] = PLACED_SHIP;   // all good - placing ship
        }
    }

    else if (active_ship.orientation == VERTICAL) {  // vertical orientation
        if (active_ship.y + active_ship.size > board_size) return 1;    // doesn't fit to board

        for (int i = 0; i < active_ship.size; ++i) {
            if (player_board[active_ship.x][active_ship.y + i] != DEFAULT) return 1;   // tile is occupied
        }

        for (int i = 0; i < active_ship.size; ++i) {
            player_board[active_ship.x][active_ship.y + i] = PLACED_SHIP;   // all good - placing ship
        }
    }
    return 0;
}


void set_fleet(SHIP active_ship, unsigned short ship_size, int flag, PLAYER *result) {
    /* Based on active_ship data (size and orientation), only one of following branches is chosen.
    Within correct branch, pointers to tiles of specific ships are saved to PLAYER struct. E.g. when Carrier
    is placed on board, two pointers point to each of its 5 tiles (**player_board and **carrier) */

    if (active_ship.orientation == HORIZONTAL) {    // determine the orientation - horizontal
        switch (ship_size) {    // find out which type of ship is active
            case 5:
                for (int i = 0; i < ship_size; ++i)     // assign each active ship pointers to struct
                    result->carrier[i] = &result->player_board[active_ship.x + i][active_ship.y];
                break;
            case 4:
                for (int i = 0; i < ship_size; ++i)
                    result->battleship[i] = &result->player_board[active_ship.x + i][active_ship.y];
                break;
            case 3:
                if (flag)
                    for (int i = 0; i < ship_size; ++i)
                        result->destroyer[i] = &result->player_board[active_ship.x + i][active_ship.y];
                else
                    for (int i = 0; i < ship_size; ++i)
                        result->submarine[i] = &result->player_board[active_ship.x + i][active_ship.y];
                break;
            default:
                for (int i = 0; i < ship_size; ++i)
                    result->patrol_boat[i] = &result->player_board[active_ship.x + i][active_ship.y];
                break;
        }
    } else if (active_ship.orientation == VERTICAL) {   // vertical orientation
        switch (ship_size) {
            case 5:
                for (int i = 0; i < ship_size; ++i)
                    result->carrier[i] = &result->player_board[active_ship.x][active_ship.y + i];
                break;
            case 4:
                for (int i = 0; i < ship_size; ++i)
                    result->battleship[i] = &result->player_board[active_ship.x][active_ship.y + i];
                break;
            case 3:
                if (flag)
                    for (int i = 0; i < ship_size; ++i)
                        result->destroyer[i] = &result->player_board[active_ship.x][active_ship.y + i];
                else
                    for (int i = 0; i < ship_size; ++i)
                        result->submarine[i] = &result->player_board[active_ship.x][active_ship.y + i];
                break;
            default:
                for (int i = 0; i < ship_size; ++i)
                    result->patrol_boat[i] = &result->player_board[active_ship.x][active_ship.y + i];
                break;
        }
    }
}


///////////////////////////////////////////////////
///////////////////// GAMEPLAY ////////////////////
///////////////////////////////////////////////////


void player_vs_player(unsigned int board_size) {
    /* PvP mode. Two PLAYER structs are created. Players alternate turns until one destroys all enemy
    ships what breaks while loop. All allocated memory will be freed when finished */

    PLAYER player1, player2;

    player1 = placement_of_ships_user(board_size);
    player2 = placement_of_ships_user(board_size);

    while (1){
        // first player
        if (player_turn(&player1, &player2, board_size)) break;

        // second player
        if (player_turn(&player2, &player1, board_size)) break;
    }

    printf(DEFAULT_COLOR"\n\tCongratulations! You just won ");
    printf(UNDERLINE_COLOR"3 points.\n"DEFAULT_COLOR);
    free_board(player1.player_board, board_size);
    free_board(player2.player_board, board_size);
}


void player_vs_computer(unsigned int board_size) {
    /* PvCPU mode. Two PLAYER structs are created. Player alternates turns with AI until one destroys all enemy
    ships what breaks while loop. Random seed is generated here. All allocated memory will be freed when finished */

    PLAYER player1, player2;
    _COORD focused_target = {0, 0};

    player1 = placement_of_ships_user(board_size);
    srand(time(NULL));
    player2 = placement_of_ships_computer(board_size);

    while (1){
        // first player
        if (player_turn(&player1, &player2, board_size)) break;

        // second player
        if (computer_turn(&player2, &player1, board_size, &focused_target)) break;
    }

    printf(DEFAULT_COLOR"\n\tCongratulations! You just won ");
    printf(UNDERLINE_COLOR"3 points.\n"DEFAULT_COLOR);
    free_board(player1.player_board, board_size);
    free_board(player2.player_board, board_size);
}


int player_turn(PLAYER* player_active, PLAYER* player_opponent, unsigned int board_size) {
    /* Pointers to active player's and opponent's struct are passed and edited. Function prints boards
    from active_player's perspective and asks him for coordinates to shoot at. This is repeated until
    valid coordinates are passed. If player hits opponent, ship_hit_check is called (checks, if that hit
    was deadly). If yes, victory condition is checked. If satisfied, returns 1. Else returns 0 */

    _COORD aim;
    int flag;

    default_screen(*player_active, *player_opponent, board_size);
    aim = get_coord();
    flag = fire(*player_opponent, aim, board_size);
    while(flag == INVALID) {    // repeats until valid coordinates are given
        printf(UNDERLINE_COLOR"\n\tInvalid shot Captain!");
        aim = get_coord();
        flag = fire(*player_opponent, aim, board_size);
    }
    player_active->last_shot = &(player_opponent->player_board[aim.x][aim.y]);

    if (flag == VALID_HIT) {    // target was hit
        if (ship_hit_check(*player_opponent))   // ship was sunk
            if (victory_check(*player_opponent)) {  // victory screen
                default_screen(*player_active, *player_opponent, board_size);
                printf(BRIGHT_RED_COLOR"\n\t##################################\n");
                printf("\t##################################\n");
                printf("\t  ------- VICTORY %s -------\n", player_active->nick);
                printf("\t##################################\n");
                printf("\t##################################\n");
                return 1;
            }
    }
    return 0;
}


_COORD get_coord() {
    /* Asks user for coordinates to shoots at and returns them as struct */

    printf(DEFAULT_COLOR"\n\n\tAhoy! Give us position to shoot at (e.g. A6): ");
    _COORD result = NULL_COORD;    // default coordinates are too big so formatting error will be detected
    char buffer[8];
    fgets(buffer, 8, stdin);
    result.x = strtol(buffer + 1, NULL, 10) - 1;   // reading coordinates
    buffer[0] = toupper(buffer[0]);
    result.y = buffer[0] - 'A';
    return result;
}


int fire(PLAYER player_opponent, _COORD aim, unsigned int board_size) {
    /* Checks whether given coordinates are valid (within board, repetitive strikes). If not, INVALID is returned.
    If shot hits water, VALID_MISS is returned and VALID_HIT is returned upon hitting ship. Function edits the board */

    // checks if within board
    if (aim.x < 0 || aim.x >= board_size) return INVALID;
    if (aim.y < 0 || aim.y >= board_size) return INVALID;
    // actually fire
    if (player_opponent.player_board[aim.x][aim.y] == DEFAULT) {
        player_opponent.player_board[aim.x][aim.y] = MISS;  // hits water
        return VALID_MISS;
    }
    else if (player_opponent.player_board[aim.x][aim.y] == PLACED_SHIP) {
        player_opponent.player_board[aim.x][aim.y] = HIT;  // hits ship
        return VALID_HIT;
    }
    else return INVALID;    // shoots where it is not allowed (repetitive strikes)
}


int ship_hit_check(PLAYER player_opponent) {
    /* Called when player hits something and checks whether it was deadly strike. Function goes through all
    enemy ships. If it finds ship with all tiles set to HIT, it returns 1. Else 0 is returned */

    int flag = 0;
    // check Carrier
    for (int i = 0; i < 5; ++i)
        if (*player_opponent.carrier[i] != HIT) {
            flag++;
            break;  // not all of them HIT
        }
    if (flag == 0) {    // newly sunk ship found (all tiles were HIT)
        for (int i = 0; i < 5; ++i) *player_opponent.carrier[i] = SUNK; // sinks the ship
        return 1;
    }
    flag = 0;
    // check Battleship
    for (int i = 0; i < 4; ++i)
        if (*player_opponent.battleship[i] != HIT) {
            flag++;
            break;  // not all of them HIT
        }
    if (flag == 0) {    // newly sunk ship found
        for (int i = 0; i < 4; ++i) *player_opponent.battleship[i] = SUNK;
        return 1;
    }
    flag = 0;
    // check Destroyer
    for (int i = 0; i < 3; ++i)
        if (*player_opponent.destroyer[i] != HIT) {
            flag++;
            break;  // not all of them HIT
        }
    if (flag == 0) {    // newly sunk ship found
        for (int i = 0; i < 3; ++i) *player_opponent.destroyer[i] = SUNK;
        return 1;
    }
    flag = 0;
    // check Submarine
    for (int i = 0; i < 3; ++i)
        if (*player_opponent.submarine[i] != HIT) {
            flag++;
            break;  // not all of them HIT
        }
    if (flag == 0) {    // newly sunk ship found
        for (int i = 0; i < 3; ++i) *player_opponent.submarine[i] = SUNK;
        return 1;
    }
    flag = 0;
    // check Patrol Boat
    for (int i = 0; i < 2; ++i)
        if (*player_opponent.patrol_boat[i] != HIT) {
            flag++;
            break;  // not all of them HIT
        }
    if (flag == 0) {    // newly sunk ship found
        for (int i = 0; i < 2; ++i) *player_opponent.patrol_boat[i] = SUNK;
        return 1;
    }
    return 0;
}


int victory_check(PLAYER player_opponent) {
    /* Goes through all opponents ships. If it finds active ship , function returns 0 (didn't win).
    If all ships are SUNK, enemy fleet is sunk (win) and functions return 1 */

    // check Carrier
    if (*player_opponent.carrier[0] != SUNK) return 0;
    // check Battleship
    if (*player_opponent.battleship[0] != SUNK) return 0;
    // check Destroyer
    if (*player_opponent.destroyer[0] != SUNK) return 0;
    // check Submarine
    if (*player_opponent.submarine[0] != SUNK) return 0;
    // check Patrol Boat
    if (*player_opponent.patrol_boat[0] != SUNK) return 0;
    return 1;
}


///////////////////////////////////////////////////
//////////////////////// AI ///////////////////////
///////////////////////////////////////////////////


int computer_turn(PLAYER* player_active, PLAYER* player_opponent, unsigned int board_size, _COORD *focused_target) {
    /* Computer tires to shoot at calculated position (recalculates when impossible). If victory conditions
    are met, 'lost scree' from opponents (user's) perspective is printed */

    _COORD aim;
    int flag;

    aim = calculate_shot(player_active, player_opponent, board_size, focused_target);
    flag = fire(*player_opponent, aim, board_size);
    while(flag == INVALID) {    // recalculates coordinates until valid shot
        aim = calculate_shot(player_active, player_opponent, board_size, focused_target);
        flag = fire(*player_opponent, aim, board_size);
    }
    player_active->last_shot = &(player_opponent->player_board[aim.x][aim.y]);

    if (flag == VALID_HIT) {
        if (ship_hit_check(*player_opponent))
            if (victory_check(*player_opponent)) {  // human is the opponent - his board goes first
                default_screen(*player_opponent, *player_active, board_size);
                printf(BRIGHT_RED_COLOR"\n\t###################################\n");
                printf("\t###################################\n");
                printf("\t     ------- YOU LOST! -------\n");
                printf("\t###################################\n");
                printf("\t###################################\n");
                return 1;
            }
    }
    return 0;
}


_COORD calculate_shot(PLAYER* player_active, PLAYER* player_opponent, unsigned int board_size, _COORD *focused_target) {
    /* Tries to find enemy ship. Function sets target (last HIT). It strikes around the current target. When two HITs
    are next to each other, functions follows the line. AI always tries to sink targeted ship. If there is no HIT
    on the board, random coordinates will be generated */

    if (player_opponent->player_board[focused_target->x][focused_target->y] != HIT)   // no active target
        *focused_target = find_last_hit(player_opponent, board_size);   // finds new target

    if (focused_target->x == 27 && focused_target->y == 27) {  // target is NULL_COORD -> there is no possible target
        *focused_target = random_shot(board_size);
        return *focused_target;     // return -> no calculations (random)
    }

    _COORD calculation = *focused_target;  // there is set target
    // firing in straight line e.g. XX~~ -> XXX~
    if (*player_active->last_shot == HIT) { // tries straight line only if last strike was hit!
        if (focused_target->x != (board_size - 1) && player_opponent->player_board[focused_target->x + 1][focused_target->y] == HIT) {
            if (line_fire_right(player_opponent, board_size, &calculation)) return calculation; // right
        }
        if (focused_target->x != 0 && player_opponent->player_board[focused_target->x - 1][focused_target->y] == HIT) {
            if (line_fire_left(player_opponent, board_size, &calculation)) return calculation;  // left
        }
        if (focused_target->y != 0 && player_opponent->player_board[focused_target->x][focused_target->y - 1] == HIT) {
            if (line_fire_up(player_opponent, board_size, &calculation)) return calculation;    // up
        }
        if (focused_target->y != (board_size - 1) && player_opponent->player_board[focused_target->x][focused_target->y + 1] == HIT) {
            if (line_fire_down(player_opponent, board_size, &calculation)) return calculation;  // down
        }
    }

    // cannot find straight line -> firing around target
    if (calculation_check(player_opponent, board_size, (short)(calculation.x + 1), (short)calculation.y) == VALID_MISS) {
        calculation.x++;    // fire right
        return calculation;
    }
    else if (calculation_check(player_opponent, board_size, (short)(calculation.x - 1), (short)calculation.y) == VALID_MISS) {
        calculation.x--;    // fire left
        return calculation;
    }
    else if (calculation_check(player_opponent, board_size, (short)calculation.x, (short)(calculation.y - 1)) == VALID_MISS) {
        calculation.y--;    // fire up
        return calculation;
    }
    else if (calculation_check(player_opponent, board_size, (short)calculation.x, (short)(calculation.y + 1)) == VALID_MISS) {
        calculation.y++;    // fire down
        return calculation;
    }
    else return random_shot(board_size);
}


int line_fire_right(PLAYER* player_opponent, unsigned int board_size, _COORD* calculation) {
    /* Follows path of successful hits in given direction and if possible edits coordinates to shoot at */

    int flag = VALID_HIT;
    _COORD new_calculation = *calculation;
    new_calculation.x++;
    while (1) {
        new_calculation.x++;    // looks one tile right while following HIT tiles
        flag = calculation_check(player_opponent, board_size, (short)new_calculation.x, (short)new_calculation.y);
        if (flag == VALID_MISS) {   // can shoot at following tile
            *calculation = new_calculation;
            return 1;
        }
        if (flag == INVALID) return 0;  // cannot strike at given direction
    }
}


int line_fire_left(PLAYER* player_opponent, unsigned int board_size, _COORD* calculation) {
    /* Follows path of successful hits in given direction and if possible edits coordinates to shoot at */

    int flag = VALID_HIT;
    _COORD new_calculation = *calculation;
    new_calculation.x--;    // looks one tile left while following HIT tiles
    while (1) {
        new_calculation.x--;
        flag = calculation_check(player_opponent, board_size, (short)new_calculation.x, (short)new_calculation.y);
        if (flag == VALID_MISS) {   // can shoot at following tile
            *calculation = new_calculation;
            return 1;
        }
        if (flag == INVALID) return 0;  // cannot strike at given direction
    }
}


int line_fire_up(PLAYER* player_opponent, unsigned int board_size, _COORD* calculation) {
    /* Follows path of successful hits in given direction and if possible edits coordinates to shoot at */

    int flag = VALID_HIT;
    _COORD new_calculation = *calculation;
    new_calculation.y--;    // looks one tile up while following HIT tiles
    while (1) {
        new_calculation.y--;
        flag = calculation_check(player_opponent, board_size, (short)new_calculation.x, (short)new_calculation.y);
        if (flag == VALID_MISS) {   // can shoot at following tile
            *calculation = new_calculation;
            return 1;
        }
        if (flag == INVALID) return 0;  // cannot strike at given direction
    }
}


int line_fire_down(PLAYER* player_opponent, unsigned int board_size, _COORD* calculation) {
    /* Follows path of successful hits in given direction and if possible edits coordinates to shoot at */

    int flag = VALID_HIT;
    _COORD new_calculation = *calculation;
    new_calculation.y++;    // looks one tile down while following HIT tiles
    while (1) {
        new_calculation.y++;
        flag = calculation_check(player_opponent, board_size, (short)new_calculation.x, (short)new_calculation.y);
        if (flag == VALID_MISS) {   // can shoot at following tile
            *calculation = new_calculation;
            return 1;
        }
        if (flag == INVALID) return 0;  // cannot strike at given direction
    }
}


int calculation_check(PLAYER* player_opponent, unsigned int board_size, short x, short y) {
    /* Checks calculated coordinates and returns, whether tile is UNKNOWN, HIT or cannot shoot there */

    if (x < 0 || x >= board_size) return INVALID;
    if (y < 0 || y >= board_size) return INVALID;
    if (player_opponent->player_board[x][y] == HIT) return VALID_HIT;
    if (player_opponent->player_board[x][y] == DEFAULT) return VALID_MISS;  // UNKNOWN for PC
    if (player_opponent->player_board[x][y] == PLACED_SHIP) return VALID_MISS;  // UNKNOWN for PC

    return INVALID;     // SUNK or MISS options
}


_COORD find_last_hit(PLAYER* player_opponent, unsigned int board_size) {
    /* Goes through opponent's board and returns coordinates of first HIT tile.
    If no HIT was found, returns NULL_COORD {27, 27} */

    _COORD result = NULL_COORD;
    for (int i = 0; i < board_size; ++i) {
        for (int j = 0; j < board_size; ++j) {
            if (player_opponent->player_board[j][i] == HIT) {   // finds hit in board
                result.x = j;
                result.y = i;
                return result;
            }
        }
    }
    return result;  // no hit was found - returns NULL_COORD {27, 27}
}


_COORD random_shot(unsigned int board_size) {
    /* Generates coordinates of random tile on enemy board */

    _COORD result;
    result.x = rand()%board_size;
    result.y = rand()%board_size;    // generates random ship position

    if (result.x%2 == result.y%2) return random_shot(board_size);   // if both coordinates are same parity -> restart
    else return result;
}

