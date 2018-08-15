deploy:
	ansible-playbook -i hosts --vault-pass .vaultpass c-flo.yml

vault:
	ansible-vault --vault-pass .vaultpass edit secrets.yml

.PHONY: deploy
