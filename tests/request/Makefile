NAME = request_test
SRC = main.cpp ../../src/core/Request/Request.cpp ../../src/core/Client.cpp ../../src/core/Utils.cpp
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

