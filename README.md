# NacreousLute

# runtime

wip. The updates are being cleaned before being made public

---

## execution split

control plane and data plane are fully separated. do not leak coordination logic into hot path.

control threads:

* job graph management
* watermark coordination
* lifecycle / teardown

data threads:

* pinned
* channel → operator → channel loop
* no dynamic allocation in steady state (if you see malloc/new in hot path, that’s a bug)

no work stealing in hot path. that decision is intentional for now (determinism > throughput). revisit after tail numbers are solid.

---

## channels

current variants:

* spsc (primary fast path)
* mpsc (input fan-in cases)

lock-free. cache-line aligned. padding enforced to avoid false sharing.

zero-copy between operators. buffers are pre-allocated. ownership transfer only.

need to:

* audit atomic memory orders (likely over-using seq_cst in a few places)
* characterize fence cost under contention
* evaluate batched push vs strict single-event push (tail impact)

numa placement not handled yet.

---

## operator model

operators exist in two tiers.

L3:

* type-erased boundary
* flexible
* slightly higher dispatch cost

L4:

* fully specialized
* CRTP + policy templates
* no virtual dispatch
* larger codegen

runtime can choose tier based on graph constraints. cost model is primitive right now (needs real measurement, not heuristics).

sliding window frameworks implemented:

* abelian group operators (general out-of-order)
* monoid operators (worst-case O(1))

need adversarial tests for heavy reordering.

---

## event time

millwheel-style watermark propagation implemented.

local watermark advancement works.
distributed reconciliation under skew is not fully validated.

adaptive watermarking (concept drift–based) not integrated yet.

clock skew handling across nodes is naïve.

---

## serialization / layout

no UB in hot path. strict aliasing respected.

layout is explicit. alignment enforced at compile time.

reinterpret_cast only used where layout is proven compatible.

still need:

* cross-arch layout verification
* endianness abstraction
* versioned schema evolution

---

## transport

runtime does not assume tcp.

transport interface is abstract.

backends:

* tcp (baseline)
* user-space (dpdk) — integration ongoing

user-space backend allows weaker reliability / ordering semantics.

backpressure across transport boundary is incomplete.
congestion control is minimal and will change.

---

## performance

currently measuring:

* p99 / p999 latency under burst load
* cache-line bouncing
* branch mispredict in operator dispatch
* atomic overhead in channel push/pop

not yet measured:

* tlb pressure
* numa locality effects
* cross-node jitter

no published numbers yet. numbers are unstable.

---

## missing

* checkpointing
* failure recovery
* deterministic replay
* distributed watermark stabilization under partition skew
* formal memory-order audit of all lock-free paths

---

if you are modifying hot path:

1. check cache alignment
2. check memory order
3. check branch predictability
4. measure before/after

otherwise don’t touch it.
