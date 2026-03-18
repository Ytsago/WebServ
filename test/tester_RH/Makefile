NAME = request_handler_test

CXX = c++
CXXFLAGS = -Wall -Wextra -std=c++98 -g -I../inc -I. -Wno-self-assign-overloaded

# Cherche les .cpp dans src/class/ ET dans src/
CLASS_SRCS = $(wildcard ../src/class/*.cpp)
SRC_SRCS = $(wildcard ../src/*.cpp)
LOCAL_SRCS = RequestHandlerTester.cpp

# Combine tous les sources
ALL_PARENT_SRCS = $(CLASS_SRCS) $(SRC_SRCS)

# Filtre main.cpp et WebServ.cpp
FILTERED_SRCS = $(filter-out ../src/class/main.cpp ../src/class/WebServ.cpp ../src/main.cpp ../src/WebServ.cpp, $(ALL_PARENT_SRCS))

SRCS = $(LOCAL_SRCS) $(FILTERED_SRCS)
OBJS = $(SRCS:.cpp=.o)

# Headers dans inc/
HEADERS = $(wildcard ../inc/*.hpp)

all: $(NAME)

$(NAME): $(OBJS)
	@echo "🔗 Linking $(NAME)..."
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "✅ Compilation réussie!"

%.o: %.cpp
	@echo "🔨 Compiling $<..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "🧹 Cleaning objects..."
	@rm -f $(OBJS)

fclean: clean
	@echo "🧹 Cleaning executable..."
	@rm -f $(NAME)

re: fclean all

run: all
	@echo "🚀 Running tests..."
	@./$(NAME)

debug:
	@echo "📋 Sources trouvées:"
	@echo "$(FILTERED_SRCS)" | tr ' ' '\n'
	@echo ""
	@echo "📁 Headers trouvés:"
	@echo "$(HEADERS)" | tr ' ' '\n'

help:
	@echo "📖 Makefile pour RequestHandler Test Suite"
	@echo ""
	@echo "Usage:"
	@echo "  make          - Compile le testeur"
	@echo "  make run      - Compile et lance les tests"
	@echo "  make debug    - Affiche les fichiers trouvés"
	@echo "  make clean    - Nettoie les .o"
	@echo "  make fclean   - Nettoie tout"
	@echo "  make re       - Recompile tout"
	@echo ""
	@echo "Structure:"
	@echo "  Headers: ../inc/*.hpp"
	@echo "  Sources: ../src/class/*.cpp ET ../src/*.cpp"

.PHONY: all clean fclean re run debug help