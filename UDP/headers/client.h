#include "constants.h"
#include "list.h"
#include "gap_buffer.h"

/* Unpacking constants */
#define CLIENT_PACKET_TYPE_MASK 0x0F
#define CLIENT_PAYLOAD_LEN_UPPER_NIBBLE_MASK 0xF00
#define CLIENT_PAYLOAD_LEN_LOWER_BYTE_MASK 0xFF

/* Ncurses constants */
#define INPUT_HEIGHT 3
#define CURSOR_START_ROW 1
#define CURSOR_START_COL 18

#define ESC_KEY_CODE 27
#define ENTER_KEY_CODE 10
#define BACKSPACE_KEY_CODE 127

/* Client structs */
typedef struct UserInArg {
    int socketFD;
    struct sockaddr_in *serverAddr;
    WINDOW *inputWindow;
    WINDOW *chatWindow;
    List *chatList;
    GapBuffer *gapBuffer;
    int *cursorPosPtr;
    int *inputIndexPtr;
    int cursorMinCol;
    int cursorMaxCol;
    int inputLength;
    int width;
    bool *exitProgramPtr;
} UserInArg;

typedef struct ServerInArg {
    int socketFD;
    WINDOW *inputWindow;
    WINDOW *chatWindow;
    List *chatList;
    GapBuffer *gapBuffer;
    int *cursorPosPtr;
    int *inputIndexPtr;
    int inputLength;
    bool *exitProgramPtr;
} ServerInArg;

/* Method signatures */
void *handle_server_input(void *arg);
void *handle_user_input(void *arg);
