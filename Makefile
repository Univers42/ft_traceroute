# ft_traceroute — 42-style Makefile
NAME    := ft_traceroute

CC      := cc
CFLAGS  := -Wall -Wextra -Werror
CPPFLAGS:= -Iinclude -D_GNU_SOURCE

SRCDIR  := src
OBJDIR  := obj
HEADER  := include/ft_traceroute.h

SRCS    := main.c args.c dns.c net.c probe.c recv.c print.c util.c ft.c
OBJS    := $(SRCS:%.c=$(OBJDIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADER) | $(OBJDIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

test: all
	@sh tests/run_all.sh

tests: test

.PHONY: all clean fclean re test tests
