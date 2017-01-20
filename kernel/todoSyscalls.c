/*
 * todoSyscalls.cpp
 *
 *  Created on: 22 ���� 2016
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
	if ((TODO_description == NULL) || (TODO_index <= 0)) {
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
	struct list_head* iterator;
	todoQueueStruct* todoItem;
	int counter = 0, isTaskFound = TASK_NOT_FOUND;
	list_for_each(iterator, &(currentTask->todoQueue)) {
		counter++;
		todoItem = list_entry(iterator, todoQueueStruct, list);
		if (counter == TODO_index) {
			isTaskFound = TASK_OK;
			break;
		}
	}
	if (isTaskFound == TASK_NOT_FOUND) {//FIXME maby, Lior's
		return -EINVAL;
	}
	if (copy_to_user(TODO_description, todoItem->_description, (todoItem->_descriptionSize)) != 0) {
		return -EFAULT;
	}
	*status = todoItem->_status;
	*TODO_deadline = todoItem->_TODO_deadline;
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
	struct list_head* iterator;
	todoQueueStruct* todoItem;
	int counter = 0, isTaskFound = TASK_NOT_FOUND;
	list_for_each(iterator, &(currentTask->todoQueue)) {
		counter++;
		todoItem = list_entry(iterator, todoQueueStruct, list);
		if (((todoItem->_TODO_deadline) < CURRENT_TIME) && ((todoItem->_status) == 0)){ //cannot delete task with passed deadline
			continue;
		} else if (counter == TODO_index) {
			isTaskFound = TASK_OK;
			break;
		}
	}
	if (isTaskFound == TASK_OK) {
		todoItem->_status = status;
		return SYS_TODO_SUCCESS;
	} else {
		return -EINVAL;
	}
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
	struct list_head* iterator;
	todoQueueStruct* todoItem;
	int counter = 0, isTaskFound = TASK_NOT_FOUND;
	list_for_each(iterator, &(currentTask->todoQueue)) {
		counter++;
		todoItem = list_entry(iterator, todoQueueStruct, list);
		if (((todoItem->_TODO_deadline) < CURRENT_TIME) && ((todoItem->_status) == 0)){ //cannot delete task with passed deadline
			continue;
		} else if (counter == TODO_index) {
			isTaskFound = TASK_OK;
			break;
		}
	}
	//FIXME Lior's method:
	//How can it be that the task won't be found?
	if (isTaskFound == TASK_OK) {
		kfree(todoItem->_description);
		list_del(iterator);
		kfree(todoItem);
		currentTask->todoQueueSize--;
		return SYS_TODO_SUCCESS;
	}
	return -EINVAL;
}

/*
 * Is PID given valid?
 * Return values- TASK_OK if it's current task or found, TASK_NOT_FOUND otherwise
 */
int isPidValid (pid_t pid) {
	if (pid == current->pid) {
		return TASK_OK;
	}
	//FIXME maby add, Lior's:
	if (pid == 1) {
		return TASK_NOT_FOUND;
	}
	struct task_struct* taskIterator = NULL;
	for (taskIterator = find_task_by_pid(pid); (taskIterator != NULL) && (taskIterator->pid > 1); taskIterator = taskIterator->p_pptr) {
		if (current->pid == taskIterator->pid) {
			return TASK_OK;
		}
	}
	return TASK_NOT_FOUND;
};

static int Perm_check(pid_t pid, pid_t original_pid)
{
	task_t* todo_task = find_task_by_pid(pid);
	if (pid == 1 || !todo_task)
	{
		return -ESRCH;
	}

	if (current->pid != pid)
	{
		return Perm_check(todo_task->p_pptr->pid, original_pid);
	}

	return 0;
}
