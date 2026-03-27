import time
import uuid

from helpers import shorten_url, run_parallel, print_report


class TestShortenLoad:
    def test_shorten_heavy_load(self):
        total_requests = 3000
        workers = 80

        def task(i):
            unique_url = f"https://example.com/load/shorten/{i}-{uuid.uuid4().hex}"
            start = time.perf_counter()
            response = shorten_url(unique_url)
            elapsed = time.perf_counter() - start
            return elapsed, response.status_code

        stats = run_parallel(total_requests, workers, task)
        print_report("shorten_heavy_load", stats)

        assert stats["failure_count"] == 0
        assert stats["success_rate"] >= 0.99
        assert stats["p95_latency_ms"] < 1000
    