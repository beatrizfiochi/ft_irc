# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: djunho <djunho@student.42porto.com>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/11/12 20:28:21 by djunho            #+#    #+#              #
#    Updated: 2026/04/24 08:56:34 by djunho           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ircserv

SRCS_DIR = .

SRCS = $(SRCS_DIR)/main.cpp \

OBJS_DIR = objs
OBJS = $(patsubst %.cpp,$(OBJS_DIR)/%.o,$(SRCS))
DEPENDS = $(patsubst %.cpp,$(OBJS_DIR)/%.d,$(SRCS))

CXFLAGS += -Wall -Werror -Wextra -std=c++98 -ggdb


# Check https://stackoverflow.com/questions/2214575/passing-arguments-to-make-run
# If the first argument is test, gdb, or valgrind
ifneq (,$(filter test gdb valgrind,$(firstword $(MAKECMDGOALS))))
  RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  $(eval $(RUN_ARGS):;@:)
endif

.PHONY: all clean fclean re debug valgrind test

all: $(NAME)

debug: CXFLAGS += -DTEST
debug: all

$(NAME): $(OBJS)
	$(CXX) $(CXFLAGS) $(OBJS) -I. -o $(NAME)

-include $(DEPENDS)

# Rule for creating object files in $(OBJS_DIR)
$(OBJS_DIR)/%.o:%.cpp Makefile | $(OBJS_DIR) $(dir $@)
	$(CXX) $(CXFLAGS) -MMD -MP -c $< -o $@

# Create the object directory if it doesn't exist
# Create the directory for the object file if it doesn't exist
$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

$(OBJS_DIR)/%:
	@mkdir -p $(dir $@)

clean:
	rm -rf $(OBJS) $(DEPENDS) $(OBJS_DIR)

fclean: clean
	rm -rf $(NAME)

re: fclean all

test: all
	./$(NAME) $(RUN_ARGS)

valgrind: all
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./$(NAME) $(RUN_ARGS)
