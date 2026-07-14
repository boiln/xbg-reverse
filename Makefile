.PHONY: test verify format format-check build deploy clean

test:
	python tests/run_tests.py

verify:
	python tests/verify_refactor.py

format:
	clang-format -i $$(find src include -type f \( -name '*.cpp' -o -name '*.hpp' \))

format-check:
	clang-format --dry-run --Werror $$(find src include -type f \( -name '*.cpp' -o -name '*.hpp' \))

build:
	python scripts/build.py --build-only

deploy:
	python scripts/build.py

clean:
	powershell -NoProfile -Command "Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue"
