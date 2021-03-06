/*
 * todoSyscalls.h
 *
 *  Created on: 22 ���� 2016
 *      Author: Eitan Levin
 */

#ifndef _TODOSYSCALLS_H_
#define _TODOSYSCALLS_H_

#include <linux/list.h>
#include <linux/types.h>
#include <linux/linkage.h>

typedef struct todoQueueStruct_t {
	int _status;
	char* _description;
	int _descriptionSize;
	struct list_head list;
	time_t _TODO_deadline;
} todoQueueStruct;

//Functions and Calls decelerations:
asmlinkage int sys_add_TODO(pid_t pid, const char* TODO_description, ssize_t description_size, time_t TODO_deadline);
asmlinkage ssize_t sys_read_TODO(pid_t pid, int TODO_index, char* TODO_description, time_t* TODO_deadline, int* status);
asmlinkage int sys_mark_TODO(pid_t pid, int TODO_index, int status);
asmlinkage int sys_delete_TODO(pid_t pid, int TODO_index);

int isPidValid (pid_t pid);



#endif /* TODOSYSCALLS_H_ */
