#define BASE_DY -0.07
#define TIMESTEP 0.01
#define SPEED_MULTIPLIER 0.01

/*
 * Type definition for a platform.
 */
typedef struct platform{
	double x; // x position (top left corner)
	double y; // y position (top left corner)
	double dy; // change in y position
	
	bool safe; // is platform safe;
	bool is_visible; // is platform visible
	
	int width; // platform width
} platform;


// ----------------------------------------------------------------
// Forward declaration of functions
// ----------------------------------------------------------------
int rand_between( int first, int last );
void setup_platform( platform*plat, int no_plats, int level);
void initialize_platforms( platform* plat, int no_plats );
void spawn_under( platform plat1, platform* plat2 );
void spawn_next( platform plat1, platform* plat2 );
bool process_platform( platform*plat, int no_plats, int level, double speed );
void draw_platforms( platform*plat, int no_plats );
void draw_single_platform( platform plat );
void platform_fall( platform* plat, int level, double speed );
void platform_fall_NG( platform* plat );
void platform_fall_G( platform* plat, int speed );

// ----------------------------------------------------------------
// Platform functions
// ----------------------------------------------------------------

/*
 *	Gets a random integer that is greater than or equal to
 *	first and less than last.
 *	Precondition: first < last;
 */
int rand_between( int first, int last ) {
	return first + rand() % ( last - first );
}

/*
 * Sets up platform positions.
 * randomly assigns a safety condition to each platform.
 */
void setup_platform( platform* plat, int no_plats, int level ) {
	
	initialize_platforms( plat, no_plats );

	for ( int i = 1; i < no_plats; i++ ) { // starts looping at second platform
		if( i%2 == 0 ){ // if platform is even
			spawn_next( plat[i-1], &(plat[i]) ); // spawns platform next to previous platform
		} else {
			spawn_under( plat[i-1], &(plat[i]) ); // spawns platform next to previous platform
		}
		plat[i].safe = ( rand_between( 0, 20 ) ) < 13; // probability of safe platform = 6/10
	}
}

/*
 * Initialises all platforms.
 * Picks a random width, safe to true
 * Position is initially set to the bottom of the screen, in the center.
 */
void initialize_platforms( platform* plat, int no_plats ){
	for ( int i = 0; i < no_plats; i++ ) {	
		plat[i].safe = true;
		plat[i].is_visible = true;
		plat[i].width = rand_between( 5, 10 ); // sets random width between 5-10 characters
		
		int x = (( screen_width() - 1) / 2 ) - ( plat[i].width / 2); // Sets x position to middle of screen
		int y = screen_height() - 4;
		
		plat[i].x = x;
		plat[i].y = y;
		plat[i].dy = BASE_DY;
	}
}


/*
 * Sets position of platform 2.
 * creates random y position that is between 5 and 10 spaces below plat1 
 * x position is random, but cannot exceed the screen width - platform width
 */
void spawn_under( platform plat1, platform* plat2 ){
	int y0 = round ( plat1. y );
	
	int x = rand_between( 0, screen_width() - plat2->width ); // offsets x position depending on first platform
	
	plat2->x = x;
	plat2->y = rand_between(y0 + 5, y0 + 10); // creates a random y position between plat1.y and 5
}

void spawn_next( platform plat1, platform* plat2 ){
	int threshold = screen_width() - ( plat1.width + plat2->width); // number of spaces the platform can occupy
	
	int offset = rand_between( 4, threshold );
	
	plat2->x = (int)( plat1.x + plat1.width + offset ) % ( screen_width() - 1 ); // x position will wrap around;
	plat2->y = plat1.y + rand_between(0, 6);
}

/*
 * Tries to move the platform; returns true if the platform moved, and false otherwise
 */
bool process_platform( platform*plat, int no_plats, int level, double speed ) {
	bool platform_moved = false;

	for ( int i = 0; i < no_plats; i++ ) {
		if ( plat[i].is_visible ){
			double x0 = round( plat[i].x ); // remembers original position
			double y0 = round( plat[i].y );

			//sprite_fall_up(platform[i]); // platform falls upwards
			platform_fall( &(plat[i]), level, speed);

			if ( plat[i].y < - 3 ) { // if platform reaches the top of the screen
				plat[i].is_visible = false; // platform becomes invisible
			}
			
			platform_moved = platform_moved || round( plat[i].x ) != x0 
											|| round( plat[i].y ) != y0;
			// platform will have moved if the new rounded positions are not the same as the original positions.
		}
	}

	return platform_moved;
}


/*
 *	Draws the platforms.
 */
void draw_platforms( platform*plat, int no_plats ) {
	for ( int i = 0; i < no_plats; i++ ) {
		if ( plat[i].is_visible ){
			draw_single_platform( plat[i] );
		}
	}
}

/*
 * Draws a single platform
 */
void draw_single_platform(platform plat){
	char character;
	
	if ( plat.safe ){
		character = '=';
	} else {
		character = 'x';
	}
	
	draw_line( plat.x, plat.y, plat.x + plat.width, plat.y, character );
	draw_line( plat.x, plat.y + 1, plat.x + plat.width, plat.y + 1, character );
}

/*
 * Makes the platform fall. Behavior depends on the level.
 */
 
 void platform_fall( platform* plat, int level, double speed ){
	 if ( level == 1 || level == 2 ){
		 platform_fall_NG( plat );
	 } else {
		 platform_fall_G( plat, speed );
	 }
 }

 
/*
 * Makes platform fall with a constant speed.
 */
void platform_fall_NG( platform* plat ){
	plat->y += BASE_DY;
}

/*
 * Makes platform fall, taking a speed multiplier into consideration
 */
void platform_fall_G( platform* plat, int speed ){
	plat->y += plat->dy * speed * SPEED_MULTIPLIER;
}