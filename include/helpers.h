// A header file for helpers.c
// Declare any additional functions in this file

/*
 * Comparator function for the process linked list
 */
int processComparator(void *process1, void *process2);

/*
 * Handle ctrl-c thing
 */
void sigint_handler();

/*
 * Handle child thing
 */
void sigchild_handler();