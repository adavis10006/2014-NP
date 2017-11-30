static struct sembuf op_lock[2] = {
	2, 0, 0, 			/* wait for [2] (lock) to equal 0 */
	2, 1, SEM_UNDO 		/* then increment [2] to 1 - this locks it */
						/* UNDO to release the lock ifprocesses exits before explicitly unlocking */
};

static struct sembuf op_endcreate[2] = {
	1, -1, SEM_UNDO,	/* decrement [1] (proc counter) with undo on exit */
						/* UNDO to adjust proc counter ifprocess exits before explicitly calling sem_close() */
	2, -1, SEM_UNDO 	/* decrement [2] (lock) back to 0 -> unlock */
};

static struct sembuf op_open[1] = {
	1, -1, SEM_UNDO 	/* decrement [1] (proc counter) with undo on exit */
};


static struct sembuf op_close[3] = {
	2, 0, 0, 			/* wait for [2] (lock) to equal 0 */
	2, 1, SEM_UNDO,		/* then increment [2] to 1 - this locks it */
	1, 1, SEM_UNDO 		/* then increment [1] (proc counter) */
};

static struct sembuf op_unlock[1] = {
	2, -1, SEM_UNDO 	/* decrement [2] (lock) back to 0 */
};

static struct sembuf op_op[1] = {
	0, 99, SEM_UNDO 	/* decrement or increment [0] with undo on exit */
						/* the 99 is set to the actual amount to add or subtract (positive or negative) */
};

int sem_create(key_t key, int initval)
{
	register int id, semval;
	if(key == IPC_PRIVATE) 
		return -1; /* not intended for private semaphores */
	else if(key == (key_t) -1) 
		return -1; /* probably an ftok() error by caller */
	again:
	if((id = semget(key, 3, 0666 | IPC_CREAT)) < 0) 
		return -1;
	if(semop(id, &op_lock[0], 2) < 0) 
		if(errno == EINVAL) goto again;
			error("can't lock", false);
	if((semval = semctl(id, 1, GETVAL, 0)) < 0) 
		error("can't GETVAL", false);
	if(semval == 0) 
	{
		semctl_arg.val = initval;
		if(semctl(id, 0, SETVAL, semctl_arg) < 0)
			error("can’t SETVAL[0]", false);
		semctl_arg.val = BIGCOUNT;
		if(semctl(id, 1, SETVAL, semctl_arg) < 0)
			error("can’t SETVAL[1]", false);
	}
	if(semop(id, &op_endcreate[0], 2) < 0)
		error("can't end create", false);
	return id;
}

void sem_rm(int id)
{
	if(semctl(id, 0, IPC_RMID, 0) < 0)
		error("can't IPC_RMID", false);
}

int sem_open(key_t key)
{
	register int id;
	if(key == IPC_PRIVATE) 
		return -1;
	else if(key == (key_t) -1) 
		return -1;
	if((id = semget(key, 3, 0)) < 0) 
		return -1; /* doesn't exist, or tables full */
	// Decrement the process counter. We don't need a lock to do this.
	if(semop(id, &op_open[0], 1) < 0) 
		error("can't open", false);
	return id;
}

void sem_close(int id)
{
	register int semval;
	// The following semop() first gets a lock on the semaphore,
	// then increments [1] - the process counter.
	if(semop(id, &op_close[0], 3) < 0) 
		error("can't semop", false);
	// ifthis is the last reference to the semaphore, remove this.
	if((semval = semctl(id, 1, GETVAL, 0)) < 0) 
		error("can't GETVAL", false);
	if(semval > BIGCOUNT) 
		error("sem[1] > BIGCOUNT", false);
	else if(semval == BIGCOUNT) 
		sem_rm(id);
	else if(semop(id, &op_unlock[0], 1) < 0) 
		error("can't unlock", false); /* unlock */
}

void sem_op(int id, int value)
{
	if((op_op[0].sem_op = value) == 0) 
		error("can't have value == 0", false);
	if(semop(id, &op_op[0], 1) < 0) 
		error("sem_op error", false);
}
void sem_wait(int id)
{
	sem_op(id, -1);
}
void sem_signal(int id)
{
	sem_op(id, 1);
}