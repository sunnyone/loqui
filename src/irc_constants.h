/* IRC constants -- This code is based on IRCatConstants.h of IRcat-Lite */
/* IRcat Constants
   1998.7   Atsushi Tagami */

#ifndef __LOQUI_IRC_CONSTANTS__
#define __LOQUI_IRC_CONSTANTS__

#define IRCCommandChar = '\001';
#define IRCModeChar    = '\007';

#define IRC_MESSAGE_PARAMETER_MAX 15

enum IRCModeFlag {
  /* user mode */
  MODE_IRC_UserInvisible             = 'i',
  MODE_IRC_UserReceivesServerNotices = 's',
  MODE_IRC_UserReceivesWallops       = 'w',
  MODE_IRC_UserOperatorPrivs         = 'o',
  /* channel mode */
  MODE_IRC_ChannelOperatorPrivs         = 'o',
  MODE_IRC_ChannelPrivateChannel        = 'p',
  MODE_IRC_ChannelSecretChannel         = 's',
  MODE_IRC_ChannelInviteOnly            = 'i',
  MODE_IRC_ChannelTopicSettable         = 't',
  MODE_IRC_ChannelNoMessagesFromOutside = 'n',
  MODE_IRC_ChannelModerated             = 'm',
  MODE_IRC_ChannelUserLimit             = 'l',
  MODE_IRC_ChannelBanMask               = 'b',
  MODE_IRC_ChannelSpeakAbility          = 'v',
  MODE_IRC_ChannelChannelKey            = 'k'
};

typedef enum {
	IRC_UNDEFINED  = 9999,
	IRC_ERR_NOSUCHNICK = 401,
	IRC_ERR_NOSUCHSERVER  = 402,
	IRC_ERR_NOSUCHCHANNEL  = 403,
	IRC_ERR_CANNOTSENDTOCHAN = 404,
	IRC_ERR_TOOMANYCHANNELS  = 405,
	IRC_ERR_WASNOSUCHNICK  = 406,
	IRC_ERR_TOOMANYTARGETS  = 407,
	IRC_ERR_NOORIGIN  = 409,
	IRC_ERR_NORECIPIENT  = 411,
	IRC_ERR_NOTEXTTOSEND  = 412,
	IRC_ERR_NOTOPLEVEL  = 413,
	IRC_ERR_WILDTOPLEVEL  = 414,
	IRC_ERR_UNKNOWNCOMMAND  = 421,
	IRC_ERR_NOMOTD   = 422,
	IRC_ERR_NOADMININFO  = 423,
	IRC_ERR_FILEERROR  = 424,
	IRC_ERR_NONICKNAMEGIVEN  = 431,
	IRC_ERR_ERRONEUSNICKNAME = 432,
	IRC_ERR_NICKNAMEINUSE  = 433,
	IRC_ERR_NICKCOLLISION  = 436,
	IRC_ERR_USERNOTINCHANNEL = 441,
	IRC_ERR_NOTONCHANNEL  = 442,
	IRC_ERR_USERONCHANNEL  = 443,
	IRC_ERR_NOLOGIN   = 444,
	IRC_ERR_SUMMONDISABLED  = 445,
	IRC_ERR_USERSDISABLED = 446,
	IRC_ERR_NOTREGISTERED  = 451,
	IRC_ERR_NEEDMOREPARAMS  = 461,
	IRC_ERR_ALREADYREGISTRED = 462,
	IRC_ERR_NOPERMFORHOST  = 463,
	IRC_ERR_PASSWDMISMATCH = 464,
	IRC_ERR_YOUREBANNEDCREEP = 465,
	IRC_ERR_KEYSET   = 467,
	IRC_ERR_CHANNELISFULL  = 471,
	IRC_ERR_UNKNOWNMODE   = 472,
	IRC_ERR_INVITEONLYCHAN  = 473,
	IRC_ERR_BANNEDFROMCHAN = 474,
	IRC_ERR_BADCHANNELKEY  = 475,
	IRC_ERR_NOPRIVILEGES  = 481,
	IRC_ERR_CHANOPRIVSNEEDED  = 482,
	IRC_ERR_CANTKILLSERVER  = 483,
	IRC_ERR_NOOPERHOST   = 491,
	IRC_ERR_UMODEUNKNOWNFLAG  = 501,
	IRC_ERR_USERSDONTMATCH  = 502,

	IRC_RPL_NONE          = 300,
	IRC_RPL_AWAY          = 301, /* */ 
	IRC_RPL_USERHOST          = 302, /* default */
	IRC_RPL_ISON          = 303, 
	IRC_RPL_UNAWAY         = 305, /* */
	IRC_RPL_NOWAWAY          = 306, /* */
	IRC_RPL_WHOISUSER   = 311, /* */
	IRC_RPL_WHOISSERVER          = 312, /* */
	IRC_RPL_WHOISOPERATOR  = 313, /* */
	IRC_RPL_WHOISIDLE          = 317, /* */
	IRC_RPL_ENDOFWHOIS          = 318, /* -- */
	IRC_RPL_WHOISCHANNELS        = 319, /* */
	IRC_RPL_WHOWASUSER          = 314, /* */
	IRC_RPL_ENDOFWHOWAS          = 369, /* -- */
	IRC_RPL_LISTSTART          = 321, /* -- */
	IRC_RPL_LIST          = 322, /* */
	IRC_RPL_LISTEND          = 323, /* -- */
	IRC_RPL_CHANNELMODEIS        = 324, /* */
	IRC_RPL_NOTOPIC          = 331, /* d */
	IRC_RPL_TOPIC = 332, /* */
	IRC_RPL_INVITING          = 341, /* */
	IRC_RPL_SUMMONING          = 342, /* d */

	IRC_RPL_VERSION          = 351, /* */
	IRC_RPL_WHOREPLY          = 352, /* */
	IRC_RPL_ENDOFWHO          = 315,
	IRC_RPL_NAMREPLY          = 353, /* */
	IRC_RPL_ENDOFNAMES          = 366,
	IRC_RPL_LINKS          = 364, /* */
	IRC_RPL_ENDOFLINKS          = 365, /* */
	IRC_RPL_BANLIST          = 367,
	IRC_RPL_ENDOFBANLIST         = 368,
	IRC_RPL_INFO          = 371, /* */
	IRC_RPL_ENDOFINFO          = 374,
	IRC_RPL_MOTDSTART          = 375,
	IRC_RPL_MOTD          = 372, /* */
	IRC_RPL_ENDOFMOTD          = 376,
	IRC_RPL_YOUREOPER          = 381, /* d */
	IRC_RPL_REHASHING          = 382, /* d */
	IRC_RPL_TIME          = 391, /* */
	IRC_RPL_USERSSTART          = 392,
	IRC_RPL_USERS          = 393, 
	IRC_RPL_ENDOFUSERS          = 394,
	IRC_RPL_NOUSERS          = 395, 
	IRC_RPL_TRACELINK          = 200,
	IRC_RPL_TRACECONNECTING      = 201,
	IRC_RPL_TRACEHANDSHAKE       = 202,
	IRC_RPL_TRACEUNKNOWN         = 203,
	IRC_RPL_TRACEOPERATOR        = 204,
	IRC_RPL_TRACEUSER          = 205,
	IRC_RPL_TRACESERVER          = 206,
	IRC_RPL_TRACENEWTYPE         = 208,
	IRC_RPL_TRACELOG          = 261,
	IRC_RPL_STATSLINKINFO        = 211,
	IRC_RPL_STATSCOMMANDS        = 212,
	IRC_RPL_STATSCLINE          = 213,
	IRC_RPL_STATSNLINE          = 214,
	IRC_RPL_STATSILINE          = 215,
	IRC_RPL_STATSKLINE          = 216,
	IRC_RPL_STATSYLINE          = 218,
	IRC_RPL_ENDOFSTATS          = 219,
	IRC_RPL_STATSLLINE          = 241,
	IRC_RPL_STATSUPTIME          = 242,
	IRC_RPL_STATSOLINE          = 243,
	IRC_RPL_STATSHLINE          = 244,
	IRC_RPL_UMODEIS          = 221,
	IRC_RPL_LUSERCLIENT          = 251,
	IRC_RPL_LUSEROP          = 252,
	IRC_RPL_LUSERUNKNOWN         = 253,
	IRC_RPL_LUSERCHANNELS        = 254,
	IRC_RPL_LUSERME          = 255,
	IRC_RPL_ADMINME          = 256,
	IRC_RPL_ADMINLOC1          = 257,
	IRC_RPL_ADMINLOC2          = 258,
	IRC_RPL_ADMINEMAIL          = 259,
	/* undocumented */
	IRC_RPL_CREATIONTIME         = 329,
	IRC_RPL_TOPICINFORMATION  = 333,
 
	/* unused? */
	IRC_RPL_TRACECLASS          = 209,
	IRC_RPL_SERVICEINFO          = 231,
	IRC_RPL_SERVICE          = 233,
	IRC_RPL_SERVLISTEND          = 235,
	IRC_RPL_WHOISCHANOP          = 316,
	IRC_RPL_CLOSING          = 362,
	IRC_RPL_INFOSTART          = 373,
	IRC_ERR_YOUWILLBEBANNED      = 466,
	IRC_ERR_NOSERVICEHOST        = 492,
	IRC_RPL_STATSQLINE          = 217,
	IRC_RPL_ENDOFSERVICES        = 232,
	IRC_RPL_SERVLIST          = 234,
	IRC_RPL_KILLDONE          = 361,
	IRC_RPL_CLOSEEND          = 363,
	IRC_RPL_MYPORTIS          = 384,
	IRC_ERR_BADCHANMASK          = 476,

	/* defined by loqui */
	IRC_COMMAND_PRIVMSG = 1002,
	IRC_COMMAND_JOIN = 1003,
	IRC_COMMAND_NOTICE = 1004,
	IRC_COMMAND_TOPIC = 1005,
	IRC_COMMAND_MODE = 1006,
	IRC_COMMAND_NICK = 1007,
	IRC_COMMAND_PART = 1008,
	IRC_COMMAND_QUIT = 1009,
	IRC_COMMAND_KICK = 1010,
	IRC_COMMAND_INVITE = 1011,
	IRC_COMMAND_ERROR = 1012,
	IRC_COMMAND_PASS = 1013,
	IRC_COMMAND_USER = 1015,
	IRC_COMMAND_OPER = 1016,
	IRC_COMMAND_SQUIT = 1017,
	IRC_COMMAND_STATS = 1018,
	IRC_COMMAND_LINKS = 1019,
	IRC_COMMAND_TIME = 1020,
	IRC_COMMAND_CONNECT = 1021,
	IRC_COMMAND_TRACE = 1022,
	IRC_COMMAND_ADMIN = 1023,
	IRC_COMMAND_INFO = 1024,
	IRC_COMMAND_WHO = 1026,
	IRC_COMMAND_WHOIS = 1027,
	IRC_COMMAND_WHOWAS = 1028,
	IRC_COMMAND_KILL = 1029,
	IRC_COMMAND_PING = 1030,
	IRC_COMMAND_PONG = 1031,
	IRC_COMMAND_AWAY = 1032,
	IRC_COMMAND_REHASH = 1033,
	IRC_COMMAND_RESTART = 1034,
	IRC_COMMAND_SUMMON = 1035,
	IRC_COMMAND_USERS = 1036,
	IRC_COMMAND_USERHOST = 1037,
	IRC_COMMAND_ISON = 1038,
	IRC_COMMAND_OBJECT = 1039,

	IRC_MESSAGE_END = 9998
} IRCResponse;



/* IRC Commands */

#define IRCCtcpVersion     "VERSION"

#define IRCCommandPrivmsg  "PRIVMSG"
#define IRCCommandJoin     "JOIN"
#define IRCCommandNotice   "NOTICE"
#define IRCCommandTopic    "TOPIC"
#define IRCCommandMode     "MODE"
#define IRCCommandNick     "NICK"
#define IRCCommandPart     "PART"
#define IRCCommandQuit     "QUIT"
#define IRCCommandKick     "KICK"
#define IRCCommandInvite   "INVITE"

#define IRCCommandError    "ERROR"

#define IRCCommandPass  "PASS"
#define IRCCommandUser  "USER"
#define IRCCommandOper  "OPER"

#define IRCCommandSQuit "SQUIT"

#define IRCCommandStats 			"STATS"
#define IRCCommandLinks				"LINKS"
#define IRCCommandTime				"TIME"
#define IRCCommandConnect			"CONNECT"
#define IRCCommandTrace				"TRACE"
#define IRCCommandAdmin		        	"ADMIN"
#define IRCCommandInfo			        "INFO"
#define IRCCommandWho				"WHO"
#define IRCCommandWhois				"WHOIS"
#define IRCCommandWhowas			"WHOWAS"
#define IRCCommandKill				"KILL"
#define IRCCommandPing				"PING"
#define IRCCommandPong				"PONG"
#define IRCCommandAway				"AWAY"
#define IRCCommandRehash			"REHASH"
#define IRCCommandRestart			"RESTART"
#define IRCCommandSummon			"SUMMON"
#define IRCCommandUsers				"USERS"
#define IRCCommandUserhost		        "USERHOST"
#define IRCCommandIson				"ISON"
#define IRCCommandObject			"OBJECT"

#define IRCMessageEnd                          "9999" /* for irc handle */

#endif  /* __LOQUI_IRC_CONSTANTS__ */
