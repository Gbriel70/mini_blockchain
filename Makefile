NAME = blockchain
FLAGS = -Wall -Wextra -Werror -fsanitize=undefined -lssl -lcrypto -Wno-deprecated-declarations
CC = cc
SRCS = $(wildcard srcs/*.c)
OBJS = $(SRCS:.c=.o)
INCLUDES = -I./includes

all: $(NAME)
$(NAME): $(OBJS)
	$(CC) $(FLAGS) $(INCLUDES) -o $(NAME) $(OBJS)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re