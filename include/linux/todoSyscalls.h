/*
 * todoSyscalls.h
 *
 *  Created on: 22 בדצמ 2016
 *      Author: Eitan Levin
 */

#ifndef TODOSYSCALLS_H_
#define TODOSYSCALLS_H_

#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/time.h>//FIXME Itamar

typedef struct todoQueueStruct_t {
	int _status;
	char* _description;
	int _descriptionSize;
	struct list_head list;
	time_t _TODO_deadline;//FIXME Itamar
	time_t deadlineInSecs;//FIXME Itamar
} todoQueueStruct;

int isPidValid (pid_t pid);

asmlinkage int sys_add_TODO(pid_t pid, const char* TODO_description, ssize_t description_size, time_t TODO_dealine);
asmlinkage ssize_t sys_read_TODO(pid_t pid, int TODO_index, char* TODO_description, time_t* TODO_deadline, int* status);
asmlinkage int sys_mark_TODO(pid_t pid, int TODO_index, int status);
asmlinkage int sys_delete_TODO(pid_t pid, int TODO_index);



#endif /* TODOSYSCALLS_H_ */
