NAME = ft_irc
CC = c++
CFLAGS = -Wall -Wextra -Werror -Wshadow -Wno-shadow -std=c++98 -g 
SRCS = ft_irc.cpp server.cpp client.cpp channel.cpp
OBJS = $(SRCS:.cpp=.o)
.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@
all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all