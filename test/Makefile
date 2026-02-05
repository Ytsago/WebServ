CC		= c++
CFLAGS	= -Wall -Wextra -Werror -std=c++98
NAME	= tester

SRCDIR	= HttpParsing/
FUNCDIR	= ../src/
OBJDIR	= .Obj/

SRCS	= $(wildcard $(SRCDIR)*.cpp) \
		  $(FUNCDIR)class/HttpParser.cpp \
		  $(FUNCDIR)class/HttpRequest.cpp \
		  $(FUNCDIR)class/HeaderMap.cpp \
		  $(FUNCDIR)utils.cpp \
		  main.cpp

OBJS	= $(addprefix $(OBJDIR), $(notdir $(SRCS:%.cpp=%.o)))

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJDIR)%.o: | $(OBJDIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I ../inc -c $(filter %/$*.cpp $*.cpp,$(SRCS)) -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

run: all
	./$(NAME)

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
