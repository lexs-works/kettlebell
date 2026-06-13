#
# config.inc - данные для подстановки
# часть генератора статического мультиязычного блога
# Kettlebell.s
#

.section .rodata
.global SITE_URL, BOT_NAME, BOT_URL, THREEMA_ID, SUPPORT_EMAIL

SITE_URL:
	.asciz "https://log.lex.sale"

BOT_NAME:
	.asciz "@lexs_gatekeeper_bot"

BOT_URL:
	.asciz "https://t.me/lexs_gatekepper_bot"

THREEMA_ID:
	.asciz "LEX7X9K"

SUPPORT_EMAIL:
	.asciz "support@lex.sale"
