# კომპილატორი და მისი საჭირო ოფციები;
# ამ ოფციების გარეშე gcc ვერ მიაგნებს fuse3-ის ფაილებს
CC=gcc
COMPILE_FLAGS=-c
FLAGS=`pkg-config fuse3 --cflags --libs`

# default target რომელსაც make ასრულებს;
# მისი სინტაქსი ასეთია:
# სახელი : მოდულების სახელების რაზეც დამოკიდებულია
# 		შესასრულებელი ბრძანება
all : Fuse.o Memcached.o util.o Directory.o chunk.o hardlink.o
	$(CC) Fuse.o Memcached.o util.o Directory.o chunk.o hardlink.o -o cachefs $(FLAGS)

# რიგითი მოდულის კონფიგურაცია:
# სახელი : დამოკიდებულებების სია (აქ შეიძლება იყოს .h ჰედერ ფაილებიც)
# 	შესასრულებელი ბრძანება
Fuse.o : Fuse.c 
	$(CC) $(COMPILE_FLAGS) -c Fuse.c $(FLAGS)

Memcached.o : Memcached.c
	$(CC) $(COMPILE_FLAGS) -c Memcached.c 

util.o : util.c
	$(CC) $(COMPILE_FLAGS) -c util.c

Directory.o : Directory.c
	$(CC) $(COMPILE_FLAGS) -c Directory.c

chunk.o : chunk.c
	$(CC) $(COMPILE_FLAGS) -c chunk.c 

hardlink.o : hardlink.c
	$(CC) $(COMPILE_FLAGS) -c hardlink.c




# დაგენერირებული არტიფაქტების წაშლა
clean :
	rm cachefs
