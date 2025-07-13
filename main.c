#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <unistd.h>
    #include <sys/ioctl.h>
    #include <termios.h>
#endif

#define DEAD '.'
#define ALIVE '*'
#define DEFAULT_COLS 80
#define DEFAULT_ROWS 24

int coords_to_index(int x, int y, int cols, int rows) {
    x = (x % cols + cols) % cols;
    y = (y % rows + rows) % rows;
    return y * cols + x;
}

char get_cell_state_at(char* grid, int x, int y, int cols, int rows) {    
    return grid[coords_to_index(x, y, cols, rows)];
}

void set_grid_cell(char* grid, int x, int y, int cols, int rows, char state) {
    grid[coords_to_index(x, y, cols, rows)] = state;
}

void init_grid(char* grid, int cols, int rows) {
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            grid[coords_to_index(x, y, cols, rows)] = DEAD;
        }
    }
}

void insert_glider(char* grid, int x, int y, int cols, int rows) {
    set_grid_cell(grid, x+1, y, cols, rows, ALIVE);
    set_grid_cell(grid, x+2, y+1, cols, rows, ALIVE);
    set_grid_cell(grid, x, y+2, cols, rows, ALIVE);
    set_grid_cell(grid, x+1, y+2, cols, rows, ALIVE);
    set_grid_cell(grid, x+2, y+2, cols, rows, ALIVE);
}

void insert_blinker(char* grid, int x, int y, int cols, int rows) {
    for (int i = -1; i <= 1; i++)
        set_grid_cell(grid, x + i, y, cols, rows, ALIVE);
}

void insert_toad(char* grid, int x, int y, int cols, int rows) {
    set_grid_cell(grid, x, y, cols, rows, ALIVE);
    set_grid_cell(grid, x+1, y, cols, rows, ALIVE);
    set_grid_cell(grid, x+2, y, cols, rows, ALIVE);
    set_grid_cell(grid, x-1, y+1, cols, rows, ALIVE);
    set_grid_cell(grid, x, y+1, cols, rows, ALIVE);
    set_grid_cell(grid, x+1, y+1, cols, rows, ALIVE);
}

void insert_beacon(char* grid, int x, int y, int cols, int rows) {
    set_grid_cell(grid, x, y, cols, rows, ALIVE);
    set_grid_cell(grid, x+1, y, cols, rows, ALIVE);
    set_grid_cell(grid, x, y+1, cols, rows, ALIVE);
    set_grid_cell(grid, x+1, y+1, cols, rows, ALIVE);
    set_grid_cell(grid, x+2, y+2, cols, rows, ALIVE);
    set_grid_cell(grid, x+3, y+2, cols, rows, ALIVE);
    set_grid_cell(grid, x+2, y+3, cols, rows, ALIVE);
    set_grid_cell(grid, x+3, y+3, cols, rows, ALIVE);
}

void insert_lwss(char* grid, int x, int y, int cols, int rows) {
    set_grid_cell(grid, x+1, y, cols, rows, ALIVE);
    set_grid_cell(grid, x+4, y, cols, rows, ALIVE);
    set_grid_cell(grid, x, y+1, cols, rows, ALIVE);
    set_grid_cell(grid, x, y+2, cols, rows, ALIVE);
    set_grid_cell(grid, x+4, y+1, cols, rows, ALIVE);
    set_grid_cell(grid, x+3, y+2, cols, rows, ALIVE);
    set_grid_cell(grid, x+1, y+3, cols, rows, ALIVE);
    set_grid_cell(grid, x+2, y+3, cols, rows, ALIVE);
    set_grid_cell(grid, x+3, y+3, cols, rows, ALIVE);
    set_grid_cell(grid, x+4, y+3, cols, rows, ALIVE);
}

void insert_block(char* grid, int x, int y, int cols, int rows) {
    set_grid_cell(grid, x, y, cols, rows, ALIVE);
    set_grid_cell(grid, x+1, y, cols, rows, ALIVE);
    set_grid_cell(grid, x, y+1, cols, rows, ALIVE);
    set_grid_cell(grid, x+1, y+1, cols, rows, ALIVE);
}

void get_terminal_size(int* cols, int* rows) {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        *cols = (csbi.srWindow.Right - csbi.srWindow.Left + 1) / 2;
        *rows = (csbi.srWindow.Bottom - csbi.srWindow.Top + 1) - 2;
    } else {
        *cols = DEFAULT_COLS;
        *rows = DEFAULT_ROWS;
    }
#else
    struct winsize w;
    if (ioctl(0, TIOCGWINSZ, &w) == 0) {
        *cols = w.ws_col / 2;
        *rows = w.ws_row - 2;
    } else {
        *cols = DEFAULT_COLS;
        *rows = DEFAULT_ROWS;
    }
#endif
}

void draw_grid(char* grid, int cols, int rows) {
    printf("\033[H");
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            printf("%c ", get_cell_state_at(grid, x, y, cols, rows));
        }
        if (y < rows - 1)
            printf("\n");
    }
    fflush(stdout);
}

int count_living_neighbors(char* grid, int x, int y, int rows, int cols) {
    int count = 0;
    for (int ofy = -1; ofy <= 1; ofy++) {
        for (int ofx = -1; ofx <= 1; ofx++) {
            if (ofy == 0 && ofx == 0) continue;  
            if (get_cell_state_at(grid, ofx + x, ofy + y, cols, rows) == ALIVE) count++;
        }
    }
    return count;
}

void compute_next_state(char* old_state, char* new_state, int cols, int rows) {
    char state;
    int neighbors;
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            state = DEAD;
            neighbors = count_living_neighbors(old_state, x, y, rows, cols);
            if (get_cell_state_at(old_state, x, y, cols, rows) == ALIVE) {
                if (neighbors == 2 || neighbors == 3) state = ALIVE;
            } else {
                if (neighbors == 3) state = ALIVE;
            }
            set_grid_cell(new_state, x, y, cols, rows, state);
        }
    }
}

void sleep_ms(int milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

int main() {
    int columns, rows;
    
    get_terminal_size(&columns, &rows);
    
    if (columns < 10) columns = 40;
    if (rows < 5) rows = 20;
    
    char* old_grid = malloc(columns * rows * sizeof(char));
    char* new_grid = malloc(columns * rows * sizeof(char));
    
    if (!old_grid || !new_grid) {
        fprintf(stderr, "Error during memory allocation.\n");
        return 1;
    }
   
    init_grid(old_grid, columns, rows);
    init_grid(new_grid, columns, rows);
   
    int center_x = columns / 2;
    int center_y = rows / 2;
    
    insert_glider(old_grid, center_x - 10, center_y - 5, columns, rows);
    insert_blinker(old_grid, center_x + 5, center_y, columns, rows);
    insert_toad(old_grid, center_x - 15, center_y + 5, columns, rows);
    insert_beacon(old_grid, center_x + 10, center_y + 5, columns, rows);
    insert_lwss(old_grid, center_x - 20, center_y - 10, columns, rows);
    insert_block(old_grid, center_x + 15, center_y - 6, columns, rows);
   
    printf("Conway's Game of Life - Ctrl+C to exit\n");
    printf("Grid dimensions: %d x %d\n", columns, rows);
    sleep_ms(2000);
   
    while (1) {
        draw_grid(old_grid, columns, rows);
        compute_next_state(old_grid, new_grid, columns, rows);
       
        char* temp = old_grid;
        old_grid = new_grid;
        new_grid = temp;
        
        sleep_ms(200);
    }
   
    free(old_grid);
    free(new_grid);
    return 0;
}