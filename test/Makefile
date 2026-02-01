CC		= c++
CFLAGS	= -Wall -Wextra -Werror -std=c++98
NAME	= tester

SRCDIR	= HttpParsing/
OBJDIR	= .Obj/

SRCS	= $(wildcard $(SRCDIR)*.cpp) ../Parsing.cpp main.cpp
OBJS	= $(SRCS:%.cpp=$(OBJDIR)%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJDIR)%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I ../inc -c $< -o $@

run: all
	./$(NAME)

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
