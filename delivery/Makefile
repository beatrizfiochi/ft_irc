
NAME = ircserv
BOT_NAME = ircbot

SRCS_DIR = .

SRCS = $(SRCS_DIR)/main.cpp \
	$(SRCS_DIR)/log.cpp \
	$(SRCS_DIR)/Server.cpp \
	$(SRCS_DIR)/Command/Command.cpp \
	$(SRCS_DIR)/Command/CommandHandler.cpp \
	$(SRCS_DIR)/Command/CommandExecute.cpp \
	$(SRCS_DIR)/Command/CommandChannelOps.cpp \
	$(SRCS_DIR)/Command/CommandJoin.cpp \
	$(SRCS_DIR)/utils/utils.cpp \
	$(SRCS_DIR)/utils/IrcParse.cpp \
	$(SRCS_DIR)/utils/Signal.cpp \
	$(SRCS_DIR)/irc.cpp \
	$(SRCS_DIR)/Client/Client.cpp \
	$(SRCS_DIR)/Channel/Channel.cpp \
	$(SRCS_DIR)/Channel/ChannelModes.cpp \

BOT_SRCS = $(SRCS_DIR)/bot/main.cpp \
	$(SRCS_DIR)/utils/utils.cpp \
	$(SRCS_DIR)/bot/Bot.cpp \
	$(SRCS_DIR)/log.cpp \
	$(SRCS_DIR)/utils/IrcParse.cpp \
	$(SRCS_DIR)/utils/Signal.cpp \

OBJS_DIR = objs
OBJS = $(patsubst %.cpp,$(OBJS_DIR)/%.o,$(SRCS))
BOT_OBJS = $(patsubst %.cpp,$(OBJS_DIR)/%.o,$(BOT_SRCS))
DEPENDS = $(patsubst %.cpp,$(OBJS_DIR)/%.d,$(SRCS) $(BOT_SRCS))

CXFLAGS += -Wall -Werror -Wextra -std=c++98

# Check https://stackoverflow.com/questions/2214575/passing-arguments-to-make-run
# If the first argument is test, gdb, or valgrind
ifneq (,$(filter test gdb valgrind,$(firstword $(MAKECMDGOALS))))
  RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  $(eval $(RUN_ARGS):;@:)
endif

.PHONY: all bonus clean fclean re debug valgrind test

all: $(NAME)

bonus: $(NAME) $(BOT_NAME)

debug: CXFLAGS += -DENABLE_LOG=1 -DENABLE_LOG_LVL=4 -ggdb
debug: all

$(NAME): $(OBJS)
	$(CXX) $(CXFLAGS) $(OBJS) -I. -o $(NAME)

$(BOT_NAME): $(BOT_OBJS)
	$(CXX) $(CXFLAGS) $(BOT_OBJS) -I. -o $(BOT_NAME)

-include $(DEPENDS)

$(OBJS_DIR)/%.o: %.cpp Makefile
	@mkdir -p $(@D)
	$(CXX) $(CXFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf $(OBJS) $(BOT_OBJS) $(DEPENDS) $(OBJS_DIR)

fclean: clean
	rm -rf $(NAME) $(BOT_NAME)

re: fclean all

test: all
	./$(NAME) $(RUN_ARGS)

valgrind: all
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./$(NAME) $(RUN_ARGS)
