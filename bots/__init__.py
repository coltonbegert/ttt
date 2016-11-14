import bots.human
import bots.random_bot
import bots.mcts
import bots.mctsplus

def get_bot(name):
    return {
        'human': human.Bot,
        'random': random_bot.Bot,
        'mcts': mcts.Bot,
        'mctsplus': mctsplus.Bot
    }[name]
