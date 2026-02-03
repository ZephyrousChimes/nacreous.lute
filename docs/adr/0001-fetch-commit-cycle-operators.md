# ADR 0001: Fetch–Commit Cycle and Delegated Cleanup Policy

## Status

Accepted

## Context

`InputGate` provides a lock-free transport mechanism between a producer thread and an operator (consumer) thread.

The operator executes on the hot path of the pipeline. Its primary responsibility is processing records and forwarding results downstream. Any additional work performed on this thread directly impacts end-to-end latency and throughput.

The system requires:

- Zero-copy record access
- Explicit backpressure
- Configurable cleanup policies
- Separation of processing work from buffer maintenance work

We must decide how to structure the fetch–commit interaction between operator and gate.

---

## Decision

We adopt a **two-phase Fetch–Commit cycle** with the following properties:

### 1. Fetch is Hot-Path and Minimal

- `fetch()` is executed on the operator (consumer) thread.
- It returns:
  - A raw pointer to a contiguous memory region
  - A record count
- No copying is performed.
- No cleanup or policy logic is executed during fetch.
- The method is designed to require minimal cycles (pointer arithmetic + index read).

This guarantees:

- Zero-copy semantics
- Minimal instruction footprint
- Cache-friendly access
- Predictable latency characteristics

---

### 2. Commit is Explicit and Lightweight

`commit()` is called by the operator to signal that records have been safely consumed and downstream output has been published.

However:

- `commit()` itself is expected to be extremely lightweight.
- It should ideally perform only index advancement and necessary memory ordering.
- Cleanup logic must not pollute the operator’s hot path.

---

### 3. Cleanup Work May Be Delegated

The actual cleanup logic (`commit_`) may be:

- Executed synchronously by `commit()`
- Deferred to the InputGate thread
- Triggered via signaling (e.g., condition variable)
- Or omitted entirely depending on policy

This allows `InputGate` to internally decide how cleanup is performed without exposing that complexity to the operator.

---

### 4. Policy Encapsulation Inside InputGate

All decisions regarding:

- Cleanup delegation
- Synchronization strategy
- Deferred processing
- Wake-up mechanisms

are encapsulated within `InputGate`.

Operators are unaware of:

- Whether commit is synchronous
- Whether commit triggers background cleanup
- Whether commit merely signals another thread

This allows different `InputGate` implementations to provide:

- Low-latency aggressive policy
- Throughput-optimized batch cleanup
- Adaptive policy
- Backpressure-aware policy

without requiring changes to operator logic.

---

## Rationale

### Why Not Perform Cleanup During Fetch?

Fetch must remain strictly minimal because:

- The operator cannot process without data.
- Fetch lies directly in the hot execution path.
- Additional work in fetch directly increases latency per record.

Therefore, fetch is optimized for:

- Zero-copy pointer return
- Minimal branching
- Minimal atomic interaction

---

### Why Not Force Operator to Perform Cleanup?

Cleanup may involve:

- Policy evaluation
- Backpressure checks
- Signaling
- Potential synchronization overhead

These operations are not record-processing work.

Performing them on the operator thread risks:

- Cache pollution
- Branch misprediction
- Increased tail latency
- Hot-path slowdown

Therefore cleanup may be delegated.

---

### Why Keep Commit Explicit?

Implicit advancement during fetch would:

- Eliminate transactional control
- Break downstream publish guarantees
- Remove flexibility for deferred commit policies

Explicit commit:

- Preserves correctness boundaries
- Enables backpressure control
- Allows transactional-style sequencing

---

## Consequences

### Positive

- Clear separation of hot path and maintenance work
- Zero-copy access
- Configurable cleanup policies
- Operator logic remains simple
- Backpressure remains explicit and controlled
- Enables future adaptive strategies

### Negative

- Slightly more complex gate implementation
- Requires careful documentation of thread roles
- Cleanup delegation introduces coordination complexity

---

## Future Considerations

Possible extensions include:

- Micro-batch commit policies
- Adaptive commit threshold based on backlog
- Producer-side wake-up heuristics
- NUMA-aware cleanup delegation (just to ensure that operator and gates are not communicating across nodes repeatedly)
- Memory ordering strategy tuning 

---

## Summary

The Fetch–Commit design enforces:

- Minimal hot-path overhead
- Explicit consumption acknowledgment
- Internalized cleanup policy
- Separation of processing and maintenance responsibilities

This preserves both performance flexibility and architectural clarity while allowing future optimization strategies without operator modification.
