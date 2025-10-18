# until we move telem service and data files to sphinx account, use the trex one
SHELL := /bin/bash

clean:
	@docker compose down

run:
	@docker compose up --build --detach
	@echo "gsw running"

logs:
	@docker compose logs --follow

bash:
	@docker compose exec ttc bash


