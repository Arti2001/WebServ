NAME = test_config
SRC = main.cpp ../../src/core/Config/VovaFiles/ServerConfig.cpp ../../src/core/Config/VovaFiles/LocationConfig.cpp ../../src/core/Config/VovaFiles/ConfigParser.cpp ../../src/core/Config/VovaFiles/ListenConfig.cpp
OBJ_DIR = obj
OBJ = $(SRC:.cpp=.o)


CXX = c++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -g -fsanitize=address

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

