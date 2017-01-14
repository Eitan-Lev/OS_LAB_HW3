/*
 * todoSyscalls.cpp
 *
 *  Created on: 22 בדצמ 2016
 *      Author: Eitan Levin
 */

#include <linux/todoSyscalls.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/current.h>
#include <linux/sched.h>
#include <linux/types.h>


#define SYS_TODO_SUCCESS 0
#define TASK_OK 1
#define TASK_NOT_FOUND 3

asmlinkage int sys_add_TODO(pid_t pid, const char* TODO_description, ssize_t description_size, time_t TODO_deadline) {
	printk(KERN_EMERG"sys_add_TODO\n");
	if ((TODO_description == NULL) || (description_size < 1) || (TODO_dealine < CURRENT_TIME)) {
		return -EINVAL;
	}
	if (isPidValid(pid) != TASK_OK) {
		return -ESRCH;
	}
	/*
	 * 	if (Perm_check(pid, pid) != 0)
		return -ESRCH;
		//TODO Lior
	 */
	struct task_struct* currentTask = current;
	if (pid != current->pid) {
		currentTask = find_task_by_pid(pid);
	}
	char* newTODODesc = (char*)kmalloc(description_size*sizeof(char), GFP_KERNEL);
	if (newTODODesc == NULL) {
		return -ENOMEM;
	}
	todoQueueStruct* newTODOItem = (todoQueueStruct*)kmalloc(sizeof(todoQueueStruct), GFP_KERNEL);
	if (newTODOItem == NULL) {
		kfree(newTODODesc);
		return -ENOMEM;
	}
	if (copy_from_user(newTODODesc, TODO_description, description_size) != 0) {
		kfree(newTODOItem);
		kfree(newTODODesc);
		return -EFAULT;
	}
	newTODOItem->_status = 0;
	newTODOItem->_description = newTODODesc;
	newTODOItem->_descriptionSize = description_size;
	newTODOItem->_TODO_deadline = TODO_deadline;
	//int taskAdded = 0;//FIXME Lior's method
	struct list_head* iterator;
	if ((currentTask->todoQueueSize) == 0) {//TODO Itamar
		list_add(&newTODOItem->list, &currentTask->todoQueue);
	} else {
		list_for_each(iterator ,&(currentTask->todoQueue)) {
			if (TODO_deadline < (list_entry(iterator, todoQueueStruct, list)->_TODO_deadline)) {
				list_add_tail(&newTODOItem->list, iterator);
				//taskAdded = 1;//FIXME Lior's method
				break;
			}
			//Itamar's method:
			if (iterator->next == (&(currentTask->todoQueue))) {
				list_add(&currentTask->list, iterator);
				break;
			}
		}
	}
	//FIXME Lior's method:
	/*if (task_added == 0) { // not added yet
		list_add_tail(&(p_elem->list), &(todo_task->todo_queue));
	}*/
	//list_add_tail(&newTODOItem->list, &currentTask->todoQueue);
	currentTask->todoQueueSize++;
	return SYS_TODO_SUCCESS;
}

asmlinkage ssize_t sys_read_TODO(pid_t pid, int TODO_index, char* TODO_description, time_t* TODO_deadline, int* status) {
	printk(KERN_EMERG"sys_read_TODO\n");
	if ((TODO_description == NULL) || (description_size < 1) || (TODO_index <= 0)) {
		return -EINVAL;
	}
	if (isPidValid(pid) != TASK_OK) {
		return -ESRCH;
	}
	struct task_struct* currentTask = current;
	if (pid != current->pid) {
		currentTask = find_task_by_pid(pid);
	}
	if (TODO_index > (currentTask->todoQueueSize)) {
		return -EINVAL;
	}
	struct list_head* listPtr;
	todoQueueStruct* todoItem;
	int counter = 0;
	list_for_each(listPtr, &(currentTask->todoQueue)) {
		counter++;
		if (counter == TODO_index) {
			todoItem = list_entry(listPtr, todoQueueStruct, list);
			break;
		}
	}
	if ((todoItem->_descriptionSize) > description_size) {
		return -EINVAL;
	}
	if (copy_to_user(TODO_description, todoItem->_description, (todoItem->_descriptionSize)) != 0) {
		return -EFAULT;
	}
	*status = todoItem->_status;
	return (todoItem->_descriptionSize);
}

asmlinkage int sys_mark_TODO(pid_t pid, int TODO_index, int status) {
	printk(KERN_EMERG"sys_mark_TODO\n");
	if (TODO_index <= 0) {
		return -EINVAL;
	}
	if (isPidValid(pid) != TASK_OK) {
		return -ESRCH;
	}
	struct task_struct* currentTask = current;
	if (pid != current->pid) {
		currentTask = find_task_by_pid(pid);
	}
	if (TODO_index > (currentTask->todoQueueSize)) {
		return -EINVAL;
	}
	struct list_head* listPtr;
	todoQueueStruct* todoItem;
	int counter = 0;
	list_for_each(listPtr, &(currentTask->todoQueue)) {
		counter++;
		if (counter == TODO_index) {
			todoItem = list_entry(listPtr, todoQueueStruct, list);
			break;
		}
	}
	todoItem->_status = status;
	return SYS_TODO_SUCCESS;
}

asmlinkage int sys_delete_TODO(pid_t pid, int TODO_index) {
	printk(KERN_EMERG"sys_delete_TODO\n");
	if (TODO_index <= 0) {
		return -EINVAL;
	}
	if (isPidValid(pid) != TASK_OK) {
		return -ESRCH;
	}
	struct task_struct* currentTask = current;
	if (pid != current->pid) {
		currentTask = find_task_by_pid(pid);
	}
	if (TODO_index > (currentTask->todoQueueSize)) {
		return -EINVAL;
	}
	struct list_head* listPtr;
	todoQueueStruct* todoItem;
	int counter = 0;
	list_for_each(listPtr, &(currentTask->todoQueue)) {
		counter++;
		if (counter == TODO_index) {
			todoItem = list_entry(listPtr, todoQueueStruct, list);
			break;
		}
	}
	kfree(todoItem->_description);
	list_del(listPtr);
	kfree(todoItem);
	currentTask->todoQueueSize--;
	return SYS_TODO_SUCCESS;
}

/*
 * Is PID given valid?
 * Return values- TASK_OK if it's current task or found, TASK_NOT_FOUND otherwise
 */
int isPidValid (pid_t pid) {
	if (pid == current->pid) {
		return TASK_OK;
	}
	struct task_struct* taskIterator = NULL;
	for (taskIterator = find_task_by_pid(pid); (taskIterator != NULL) && (taskIterator->pid > 1); taskIterator = taskIterator->p_pptr) {
		if (current->pid == taskIterator->pid) {
			return TASK_OK;
		}
	}
	return TASK_NOT_FOUND;
};

