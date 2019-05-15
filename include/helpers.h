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
 * Return ProcessEntry struct with corresponding pid
 */
ProcessEntry_t* getByPid(pid_t pid, List_t* bg_list);

/*
 * Remove terminated processes from linked list
 */
// void removeTerminatedFromList(List_t* bg_list);

/*
 * Remove back process with given pid
 */
// void removeBackProcess(pid_t pid, List_t* bg_list);

/*
 * Return 1 if pid is a background process (in linkedlist), otherwise return 0
 */
int isBackgroundProcess(pid_t pid, List_t* bg_list);

/*
 * Handle ctrl-c thing
 */
void sigint_handler();

/*
 * Handle child thing
 */
void sigchild_handler();