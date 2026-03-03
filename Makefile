# -----------RULES-----------#

CFLAGS = -Wall -Wextra -Werror -MMD -MP -g3 -std=c++98
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

MAIN =	main.cpp							utils.cpp		

CLASS = ConfigParser.cpp					LocationConfig.cpp			\
		ServerConfig.cpp					WebServ.cpp					\
		AMessage.cpp						Request.cpp					\
		Server.cpp							Client.cpp					\
		ANetContainer.cpp					Response.cpp				\
		CgiHandler.cpp						RequestHandler.cpp			\
		FileHandler.cpp						CgiContainer.cpp			\
		StatusCode.cpp						HttpRequest.cpp				\
		HttpParser.cpp						HeaderMap.cpp				\
		Logger.cpp

INC = ConfigParser.hpp						LocationConfig.hpp			\
	ServerConfig.hpp						ConfigException.hpp			\
	WebServ.hpp								AMessage.hpp				\
	ANetContainer							Recipient.hpp				\
	Request.hpp								Client.hpp					\
	Server.hpp								Response.hpp				\
	CgiHandler.hpp							RequestHandler.hpp			\
	FileHandler.hpp							CgiContainer.hpp			\
	StatusCode.hpp							HttpRequest.hpp				\
	HttpParser.hpp							HeaderMap.hpp				\
	Logger.hpp								color.h

# -----------SRCS-----------#

SRCS =	$(addprefix $(SRCDIR), $(MAIN)) \
		$(addprefix $(CLSSDIR), $(CLASS))

# -----------OTHER-----------#

OBJS =	$(patsubst $(SRCDIR)%.cpp, $(OBJDIR)%.o, $(SRCS))

DEPS =	$(OBJS:.o=.d)

HEADER = $(addprefix $(INCDIR), $(INC))

LIBS =	

NAME = WebServ

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

test:
	$(MAKE) run -C ./test/

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

.PHONY: clean fclean re all bonus test
