import os
import time
import statistics
from concurrent.futures import ThreadPoolExecutor, as_completed

import requests


BASE_URL = os.getenv("APP_BASE_URL", "http://localhost:8080")


def percentile(sorted_values, p):
    if not sorted_values:
        return 0.0
    if len(sorted_values) == 1:
        return sorted_values[0]

    k = (len(sorted_values) - 1) * p
    f = int(k)
    c = min(f + 1, len(sorted_values) - 1)

    if f == c:
        return sorted_values[f]

    d0 = sorted_values[f] * (c - k)
    d1 = sorted_values[c] * (k - f)
    return d0 + d1


def wait_until_ready(timeout_seconds=30):
    deadline = time.time() + timeout_seconds
    last_error = None

    while time.time() < deadline:
        try:
            response = requests.get(f"{BASE_URL}/health", timeout=2)
            if response.status_code == 200:
                return
        except requests.RequestException as exc:
            last_error = exc

        time.sleep(1)

    raise RuntimeError(f"service did not become ready in time: {last_error}")


def create_user(username: str) -> int:
    response = requests.post(
        f"{BASE_URL}/users",
        json={"username": username},
        timeout=5,
    )
    response.raise_for_status()
    return response.json()["id"]


def shorten_url(url: str, user_id=None):
    body = {"url": url}
    if user_id is not None:
        body["user_id"] = user_id

    response = requests.post(
        f"{BASE_URL}/shorten",
        json=body,
        timeout=10,
    )
    return response


def resolve_short_key(short_key: str):
    return requests.get(
        f"{BASE_URL}/{short_key}",
        allow_redirects=False,
        timeout=10,
    )


def run_parallel(total_requests, workers, task_fn):
    latencies = []
    status_codes = []
    failures = 0

    start = time.perf_counter()

    with ThreadPoolExecutor(max_workers=workers) as executor:
        futures = [executor.submit(task_fn, i) for i in range(total_requests)]

        for future in as_completed(futures):
            try:
                elapsed, status_code = future.result()
                latencies.append(elapsed)
                status_codes.append(status_code)
            except Exception:
                failures += 1

    total_time = time.perf_counter() - start
    latencies_sorted = sorted(latencies)

    success_count = sum(1 for code in status_codes if 200 <= code < 400)
    total_done = len(status_codes) + failures

    return {
        "total_requests": total_requests,
        "completed_requests": total_done,
        "success_count": success_count,
        "failure_count": failures,
        "success_rate": (success_count / total_done) if total_done else 0.0,
        "total_time_sec": total_time,
        "rps": (total_done / total_time) if total_time > 0 else 0.0,
        "avg_latency_ms": (statistics.mean(latencies_sorted) * 1000.0) if latencies_sorted else 0.0,
        "p95_latency_ms": percentile(latencies_sorted, 0.95) * 1000.0,
        "p99_latency_ms": percentile(latencies_sorted, 0.99) * 1000.0,
        "status_codes": status_codes,
    }


def print_report(name, stats):
    print()
    print(f"=== {name} ===")
    print(f"total_requests:   {stats['total_requests']}")
    print(f"completed:        {stats['completed_requests']}")
    print(f"success_count:    {stats['success_count']}")
    print(f"failure_count:    {stats['failure_count']}")
    print(f"success_rate:     {stats['success_rate']:.4f}")
    print(f"total_time_sec:   {stats['total_time_sec']:.3f}")
    print(f"rps:              {stats['rps']:.2f}")
    print(f"avg_latency_ms:   {stats['avg_latency_ms']:.2f}")
    print(f"p95_latency_ms:   {stats['p95_latency_ms']:.2f}")
    print(f"p99_latency_ms:   {stats['p99_latency_ms']:.2f}")
