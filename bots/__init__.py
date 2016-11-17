import bots.human
import bots.random_bot
import bots.mctscomplex
import bots.mctspure
import bots.interruptable
import bots.cpybot

def get_bot(name):
    return {
        'human': human.Bot,
        'random': random_bot.Bot,
        'mcts': mctspure.Bot,
        'mctsplus': mctscomplex.Bot,
        'interrupt': interruptable.Bot,
        'fast': cpybot.Bot
    }[name]
