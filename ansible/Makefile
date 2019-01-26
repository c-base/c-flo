deploy:
	ansible-playbook -i hosts --vault-pass .vaultpass c-flo.yml

vault:
	ansible-vault --vault-pass .vaultpass edit secrets.yml

35c3:
	ansible-playbook -i hosts --vault-pass .vaultpass c-flo.yml -l north-display,west-display,east-display,south-display,c-lab-display,mainhall-display,soundlab-display,weltenbau-display

c-flo:
	ansible-playbook -i hosts --vault-pass .vaultpass c-flo.yml -l c-flo

.PHONY: deploy 35c3 c-flo vault deploy
