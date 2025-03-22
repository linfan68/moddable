import Timer from 'timer'

trace('Hello, Linux Embedded!!!\n')

// !!! Abort hook does NOT work !!!
globalThis.abort = function (msg, exception) {
	trace(`abort hook called with "${msg}"\n`);
	trace(exception.stack, "\n");

	if (("unhandled exception" === msg) ||
		("unhandled rejection" === msg))
		return false;		// do not abort
}

//
// Error Handling Tests
// !!! ONLY ENABLE ONE CASE AT A TIME !!!
//

// ***************************************************************************
// Case 1: Uncaught exception from top level
// ***************************************************************************
// throw new Error('Uncaught exception from top level!')

// =====OUTPUT======
// Hello, Linux Embedded!!!
// Error: unhandled exception
// =================
// (process exits)

// ***************************************************************************
// Case 2: Uncaught exception from setTimeout
// ***************************************************************************
// Timer.set(() => {
// 	throw new Error('Uncaught exception from setTimeout!')
// }, 1000)

// =====OUTPUT======
// Hello, Linux Embedded!!!
// Error: unhandled exception
// =================
// Process exited with code


// ***************************************************************************
// Case 3: Promise rejection
// ***************************************************************************
// Promise.reject(new Error('Promise rejection!'))

// =====OUTPUT======
// Hello, Linux Embedded!!!
// =================
// Process keeps running

// ***************************************************************************
// Case 4: Uncaught exception from Promise
// ***************************************************************************
// new Promise(() => {
// 	throw new Error('Uncaught exception from Promise!')
// })

// =====OUTPUT======
// Hello, Linux Embedded!!!
// =================
// Process keeps running

// ***************************************************************************
// Case 5: Uncaught exception from setTimeout in Promise
// ***************************************************************************
// new Promise(() => {
// 	Timer.set(() => {
// 		throw new Error('Uncaught exception from setTimeout in Promise!')
// 	}, 1000)
// })

// =====OUTPUT======
// Hello, Linux Embedded!!!
// Error: unhandled exception
// =================
// Process exited

// ***************************************************************************
// Case 6: Promise rejection in setTimeout
// ***************************************************************************
// new Promise((resolve, reject) => {
// 	Timer.set(() => {
// 		reject(new Error('Promise rejection in setTimeout!'))
// 	}, 1000)
// })

// =====OUTPUT======
// Hello, Linux Embedded!!!
// =================
// Process keeps running