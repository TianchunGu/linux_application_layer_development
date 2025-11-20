CC:=gcc

.PHONY: named_fifo named_fifo_cleanm p_c_mq pc_mq_clean

system_test: system_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

fork_test: fork_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

fork_fd_test: fork_fd_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

erlou: erlou.c
	-$(CC) -o $@ $^

execve_test: execve_test.c erlou
	-$(CC) -o $@ $<
	-./$@
	-rm ./$@ ./$(word 2, $^)

fork_execve_test: fork_execve_test.c erlou
	-$(CC) -o $@ $(firstword $^)
	-./$@
	-rm ./$@ ./$(lastword $^)

waitpid_test: waitpid_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

pstree_test: pstree_test.c erlou
	-$(CC) -o $@ $<
	-./$@
	-rm ./$@ ./$(lastword $^)

erlou_block: erlou_block.c
	-$(CC) -o $@ $^

orphan_process_test: orphan_process_test.c erlou_block
	-$(CC) -o $@ $<
	-./$@
	-rm ./$@ ./$(word 2, $^)

perror_test: perror_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

errno_test: errno_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

unnamed_pipe_test: unnamed_pipe_test.c
	-$(CC) -o $@ $<
	-./$@ "test"
	-rm ./$@

fifo_write: fifo_write.c
	$(CC) -o $@ $<

fifo_read: fifo_read.c
	$(CC) -o $@ $<

named_fifo: fifo_write fifo_read

named_fifo_clean:
	-rm ./fifo_write ./fifo_read

shared_memory: shared_memory.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

shared_memory_block: shared_memory_block.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

father_son_mq_test: father_son_mq_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

producer: producer.c
	$(CC) -o $@ $^

cosumer: consumer.c
	$(CC) -o $@ $^

p_c_mq: producer consumer

pc_mq_clean:
    -rm ./producer ./consumer