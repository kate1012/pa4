CC=gcc
CFLAGS= -Wall -O -g

search: src/main.c sorted_list.o indexer.o index_parser.o util.o
	$(CC) $(CFLAGS) src/main.c bin/sorted_list.o bin/index_parser.o bin/util.o bin/indexer.o -o search

indexer: src/indexer_main.c sorted_list.o indexer.o tokenizer.o
	$(CC) $(CFLAGS) src/indexer_main.c bin/sorted_list.o bin/indexer.o bin/tokenizer.o -o indexer

tokenizer.o: src/tokenizer.c src/tokenizer.h
	$(CC) $(CFLAGS) -o bin/tokenizer.o -c src/tokenizer.c

indexer.o: src/indexer.c src/indexer.h
	$(CC) $(CFLAGS) -o bin/indexer.o -c src/indexer.c

index_parser.o: src/index_parser.c src/index_parser.h
	$(CC) $(CFLAGS) -o bin/index_parser.o -c src/index_parser.c

sorted_list.o: src/sorted_list.c src/sorted_list.h
	$(CC) $(CFLAGS) -o bin/sorted_list.o -c src/sorted_list.c

util.o: src/util.c src/util.h
	$(CC) $(CFLAGS) -o bin/util.o -c src/util.c

clean:
	rm -rf bin/*.o search indexer
