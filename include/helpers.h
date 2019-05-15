// A header file for helpers.c
// Declare any additional functions in this file

/*
 * Comparator function for the process linked list
 */
int processComparator(void *process1, void *process2);

/*
 * Create a node from the passed command and add to linked list
 */
void addBackProcess(char* buffer, List_t* bg_list);

/*
 * Remove all nodes with terminated child processes (zombie processes)
 */
void clearZombies(pid_t pid, List_t* bg_list);

/*
 * Handle ctrl-c thing
 */
void sigint_handler();

/*
 * Handle child thing
 */
void sigchild_handler();