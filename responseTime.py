import math

class Task:
	def __init__(self, p, c, t, b):
	    self.P = p
	    self.C = c
	    self.T = t
	    self.B = b


t2 =  Task(4, 0.0013, 3.3, 0)
oc =  Task(4, 0.0017, 3.3, 0)
dec = Task(3, 0.101524, 80, 0.7561) 
act = Task(3, 0.002, 80, 0)
sw =  Task(2, 0.004, 350, 0.756)
acq = Task(2, 0.0113365, 100, 0.755)
key = Task(2, 0.005046, 100, 0.7552)
bt =  Task(2, 0.0017, 150, 0.7535)
prt = Task(1, 0.7525, 500, 0)

allTasks = [t2, oc, dec, act, sw, acq, key, bt, prt]


def calcInterf(task, lastR):
    res = 0
    for t in allTasks:
        if t.P > task.P:
            res += math.ceil(lastR/t.T)*t.C
    return res

def calcResponseTime(task):
    i = 0
    r = 0
    old_r = 0
        
    while 1:
        old_r = r
        if i == 0:
            r = task.C+task.B
            i += 1
        else:
            r = task.C + task.B + calcInterf(task, r)
            i += 1
        if old_r == r:
            return r

def run(task):
    rspTime = calcResponseTime(task)
    if rspTime < task.T:
        print("Passed with response: " + str(rspTime) + " D: " + str(task.T))
    else:
        print("Not Passed with response: " + str(rspTime) + " D: " + str(task.T))


run(t2)
run(oc)
run(dec)
run(act)
run(sw)
run(acq)
run(key)
run(bt)
run(prt)

