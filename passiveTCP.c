
int	passivesock(int port, const char *transport,
		int qlen);

int
passiveTCP(int port, int qlen)
{
	return passivesock(port, "tcp", qlen);
}
