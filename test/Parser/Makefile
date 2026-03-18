CXXFLAG	= -Wall -Wextra -Werror -std=c++98
DEPFLAG	= -MMD 
ARFLAG	= -rcs

FLAG	= $(CXXFLAG) $(DEPFLAG)

CXX		= c++
AR		= ar

SRCDIR	= src/
OBJDIR	= .obj/
INCDIR	= inc/

SRC		= ActionParser.cpp	BasicParsers.cpp	Combinator.cpp \
		  Parser.cpp

SRCS	= $(addprefix $(SRCDIR), $(SRC)) \
		  $(MAIN)

OBJS	= $(patsubst $(SRCDIR)%.cpp, $(OBJDIR)%.o, $(SRCS))
DEPS	= $(OBJS:.o=.d)

NAME	= libParser.a

all: $(NAME)

test: $(NAME)
	$(CXX) parser_tests.cpp $(NAME) -I $(INCDIR) -o tester
	./tester

$(NAME): $(OBJS)
	$(AR) $(ARFLAG) $(NAME) $(OBJS)

$(OBJDIR)%.o: $(SRCDIR)%.cpp Makefile | $(OBJDIR)
	$(CXX) $(FLAG) -I $(INCDIR) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)
clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -rf $(NAME)
	rm -rf tester

re: fclean all

.PHONY: clean fclean all re test
