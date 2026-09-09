NAME = codexion
CC = cc -fsanitize=address -g
CFLAGS = -Wall -Wextra -Werror -pthread
SRC = main.c parsing.c init.c utils.c heap.c dongle.c coder.c monitor.c logger.c
OBJ = $(SRC:.c=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.c codexion.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all
