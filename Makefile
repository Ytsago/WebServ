# -----------RULES-----------#

CFLAGS = -Wall -Wextra -MMD -MP -std=c++98
CC = c++
AR = ar
ARFLAG = -rcs

# -----------PATHS-----------#

SRCDIR = src/
CLSSDIR = $(SRCDIR)class/
INCDIR = inc/
LIBDIR =
OBJDIR = .Obj/

# -----------FILES-----------#

MAIN =	main.cpp

CLASS = ConfigParser.cpp						LocationConfig.cpp			\
		ServerConfig.cpp

INC = ConfigParser.hpp						LocationConfig.hpp			\
	ServerConfig.hpp

# -----------SRCS-----------#

SRCS =	$(addprefix $(SRCDIR), $(MAIN)) \
		$(addprefix $(CLSSDIR), $(CLASS))

# -----------OTHER-----------#

OBJS =	$(patsubst $(SRCDIR)%.cpp, $(OBJDIR)%.o, $(SRCS))

DEPS =	$(OBJS:.o=.d)

HEADER = $(addprefix $(INCDIR), $(INC))

LIBS =	

NAME = Whatever 

# -----------RULES-----------#

all: $(NAME)

$(NAME): $(LIBS) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) $(LIBS)

$(OBJDIR)%.o: $(SRCDIR)%.cpp Makefile | $(OBJDIR)
	$(CC) $(CFLAGS) -I $(INCDIR) $(if $(LIBS),-I $(LIBDIR)$(INCDIR)) -c $< -o $@ 

$(OBJDIR):
	mkdir -p $(OBJDIR) $(dir $(OBJS))

$(LIBS): FORCE
	@$(MAKE) -C $(LIBDIR) --no-print-directory

# -----------UTILS-----------#

run: all
	./$(NAME)

nc:
	nc localhost 8080

clean:
	rm -rf $(OBJDIR)
ifneq ($(LIBS),)
	@$(MAKE) $@ -C $(LIBDIR) 
endif

fclean: clean
	rm -f $(NAME)
ifneq ($(LIBS),)
	@$(MAKE) $@ -C $(LIBDIR) 
endif

re: fclean all

FORCE:

-include $(DEPS)

.PHONY: clean fclean re all bonus
