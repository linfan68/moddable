import Timer from 'timer'

trace('Hello, Linux Embedded!!!\n')

Timer.set(id => trace('immediate\n'))

Timer.set(id => trace('oneshot\n'), 1000)

let count = 0
Timer.repeat(id => {
	trace(`repeat ${count} \n`)
	5 == ++count && Timer.clear(id)
}, 1000)
