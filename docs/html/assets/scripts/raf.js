/**
 * ## Mock Request Animation Frame (RAF)
 * A simple mock for requestAnimationFrame testing with fake timers.
 * This is a fork of the mock-raf package provided by Alex Lande which
 * can be found here. This fork implements request IDs, which can be
 * used e.g. to test if the right animation frames are cancelled.
 *
 * License MIT
 *
 * @usage
 * ```
 * let createMockRaf = require('mock-raf');
 * let mockRaf = createMockRaf();
 *
 * // Stub out your `requestAnimationFrame` method.
 * sinon.stub(window, 'requestAnimationFrame', mockRaf.raf);
 *
 * // Take 10 `requestAnimationFrame` steps (your callback will fire 10 times)
 * mockRaf.step(10);
 * ```
 *
 * @see https://libraries.io/npm/mockraf/0.1.0
 *
 * @returns {{cancel: cancel, raf: (function(*): number), now: (function(): number), step: step}}
 */
function createMockRaf() {
    let allCallbacks = {};
    let callbacksLength = 0;
    let prevTime = 0;
    /**
     * ### now()
     * Returns the current now value of the mock. Starts at 0 and
     * increases with each step() taken. Useful for stubbing out
     * performance.now() or a polyfill when using
     * requestAnimationFrame with timers.
     *
     * @returns {number}
     */
    let now = function() {
        return prevTime;
    };
    let raf = function(callback) {
        callbacksLength += 1;
        allCallbacks[callbacksLength] = callback;
        return callbacksLength;
    };
    let cancel = function(id) {
        delete allCallbacks[id];
    };
    /**
     * ### step(options)
     * Takes requestAnimationFrame steps.
     *
     * Fires currently queued callbacks for each step and
     * increments now time for each step. The primary way
     * to interact with a mockRaf instance for testing.
     *
     * @param opts
     */
    let step = function(opts) {
        let options = Object.assign({}, { time: 1000 / 60, count: 1 }, opts);
        
        let oldAllCallbacks;
        for (let i = 0; i < options.count; i++) {
            oldAllCallbacks = allCallbacks;
            allCallbacks = {};
            Object.keys(oldAllCallbacks).forEach(
                function(id) {
                    let callback = oldAllCallbacks[id];
                    callback(
                        prevTime +
                        options.time
                    );
                }
            );
            prevTime += options.time;
        }
    };
    
    return { now: now, raf: raf, cancel: cancel, step: step };
}
