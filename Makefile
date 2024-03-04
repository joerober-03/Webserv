#COLORS
GREEN		= \033[0;32m
RED 		= \033[0;31m
BOLD		= \033[1m
NORMAL		= \033[0m
UP 			= \033[A
CUT 		= \033[K

#OUTPUT
NAME		= webserv

#FILES
HEADS_LIST	= webserv.hpp servers.hpp
HEADS_DIR	= ./incs/
HEADS		= $(addprefix $(HEADS_DIR), $(HEADS_LIST))

SRCS_DIR	= ./srcs/
SRCS_FILES	= main.cpp requestHandlers.cpp cgi.cpp error.cpp parsing.cpp servers.cpp utils.cpp
SRCS		:= ${addprefix ${SRCS_DIR}, ${SRCS_FILES}}

OBJS_DIR	= ./objs/
OBJS_FILES	:= ${SRCS_FILES:.cpp=.o}
OBJS		:= ${addprefix ${OBJS_DIR}, ${OBJS_FILES}}

#COMMANDS
CC			= c++
CFLAGS		= -Wall -Wextra -Werror -std=c++98
AR			= ar rcs
MKDIR		= mkdir -p
RM			= rm -rf
LIBS		= -lft -L$(LIBFT_DIR)
INCS		= -I$(HEADS_DIR)

all: ${NAME}

#Compile normal executable
${NAME}: ${OBJS_DIR} ${OBJS}
	@${CC} ${CFLAGS} ${INCS} ${OBJS} -o ${NAME}
	@echo "$(GREEN)[$(BOLD)OK$(NORMAL)]$(GREEN)$(NORMAL) created and compiled object files"
	@echo "$(GREEN)[$(BOLD)OK$(NORMAL)]$(GREEN)$(NORMAL) $(BOLD)$(NAME)$(NORMAL) is ready"

#Create objects directory
${OBJS_DIR}:
	@${MKDIR} ${OBJS_DIR}

#Compile normals objects
${OBJS_DIR}%.o: ${SRCS_DIR}%.cpp
	@echo "$(RED)[$(BOLD)Compiling$(NORMAL)$(RED)]$(NORMAL) $<$(UP)$(CUT)"
	@${CC} ${CFLAGS} ${INCS} -o $@ -c $<

#Clean obj files
clean:
	@${RM} ${OBJS_DIR}
	@echo "$(GREEN)[$(BOLD)OK$(NORMAL)$(GREEN)]$(NORMAL) object files deleted"

#Clean objs files and name
fclean:	clean
	@${RM} ${NAME}
	@echo "$(GREEN)[$(BOLD)OK$(NORMAL)$(GREEN)]$(NORMAL) $(BOLD)$(NAME)$(NORMAL) deleted"

re: fclean all

.PHONY: all clean fclean re
