// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo
/*******************************************************************************

     Mouser

     Driver by Frank Palazzolo (palazzol@comcast.net)

    - This driver was done with only flyer shots to go by.
    - Colors are a good guess (might be perfect)
    - Clock and interrupt speeds for the sound CPU is a guess, but seem
      reasonable, especially because the graphics seem to be synched
    - Sprite priorities are unknown

*******************************************************************************/

#include "emu.h"
#include "includes/mouser.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "screen.h"
#include "speaker.h"


/* Mouser has external masking circuitry around
 * the NMI input on the main CPU */
WRITE_LINE_MEMBER(mouser_state::nmi_enable_w)
{
	m_nmi_enable = state;
	if (!m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

WRITE_LINE_MEMBER(mouser_state::mouser_nmi_interrupt)
{
	if (state && m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

/* Sound CPU interrupted on write */

WRITE8_MEMBER(mouser_state::mouser_sound_nmi_clear_w)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

INTERRUPT_GEN_MEMBER(mouser_state::mouser_sound_nmi_assert)
{
	if (m_nmi_enable)
		device.execute().set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void mouser_state::mouser_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x6bff).ram();
	map(0x8800, 0x88ff).nopw(); /* unknown */
	map(0x9000, 0x93ff).ram().share("videoram");
	map(0x9800, 0x9bff).ram().share("spriteram");
	map(0x9c00, 0x9fff).ram().share("colorram");
	map(0xa000, 0xa000).portr("P1");
	map(0xa000, 0xa007).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0xa800, 0xa800).portr("SYSTEM");
	map(0xb000, 0xb000).portr("DSW");
	map(0xb800, 0xb800).portr("P2").w("soundlatch", FUNC(generic_latch_8_device::write)); /* byte to sound cpu */
}

void mouser_state::decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x5fff).rom().share("decrypted_opcodes");
}

void mouser_state::mouser_sound_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x2000, 0x23ff).ram();
	map(0x3000, 0x3000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x4000, 0x4000).w(this, FUNC(mouser_state::mouser_sound_nmi_clear_w));
}

void mouser_state::mouser_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("ay1", FUNC(ay8910_device::data_address_w));
	map(0x80, 0x81).w("ay2", FUNC(ay8910_device::data_address_w));
}

static INPUT_PORTS_START( mouser )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Difficulty ) )       /* guess ! - check code at 0x29a1 */
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x04, "40000" )
	PORT_DIPSETTING(    0x08, "60000" )
	PORT_DIPSETTING(    0x0c, "80000" )
	PORT_DIPNAME( 0x70, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,     /* 8*8 characters */
	1024,    /* 1024 characters */
	2,       /* 2 bits per pixel */
	{ 8192*8, 0 },
	{0, 1, 2, 3, 4, 5, 6, 7},
	{8*0, 8*1, 8*2, 8*3, 8*4, 8*5, 8*6, 8*7},
	8*8
};


static const gfx_layout spritelayout =
{
	16,16,   /* 16*16 characters */
	64,      /* 64 sprites (2 banks) */
	2,       /* 2 bits per pixel */
	{ 8192*8, 0 },
	{0,  1,  2,  3,  4,  5,  6,  7,
		64+0, 64+1, 64+2, 64+3, 64+4, 64+5, 64+6, 64+7},
	{8*0, 8*1, 8*2, 8*3, 8*4, 8*5, 8*6, 8*7,
		128+8*0, 128+8*1, 128+8*2, 128+8*3, 128+8*4, 128+8*5, 128+8*6, 128+8*7},
	16*16
};


static GFXDECODE_START( mouser )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout,       0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0x1000, spritelayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0x1800, spritelayout,     0, 16 )
GFXDECODE_END


void mouser_state::machine_start()
{
	save_item(NAME(m_nmi_enable));
}

void mouser_state::machine_reset()
{
}

MACHINE_CONFIG_START(mouser_state::mouser)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)   /* 4 MHz ? */
	MCFG_CPU_PROGRAM_MAP(mouser_map)
	MCFG_CPU_OPCODES_MAP(decrypted_opcodes_map)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000)  /* ??? */
	MCFG_CPU_PROGRAM_MAP(mouser_sound_map)
	MCFG_CPU_IO_MAP(mouser_sound_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(mouser_state, mouser_sound_nmi_assert,  4*60) /* ??? This controls the sound tempo */

	MCFG_DEVICE_ADD("mainlatch", LS259, 0) // type unconfirmed
	MCFG_ADDRESSABLE_LATCH_Q0_OUT_CB(WRITELINE(mouser_state, nmi_enable_w))
	MCFG_ADDRESSABLE_LATCH_Q1_OUT_CB(WRITELINE(mouser_state, flip_screen_x_w))
	MCFG_ADDRESSABLE_LATCH_Q2_OUT_CB(WRITELINE(mouser_state, flip_screen_y_w))

	MCFG_GENERIC_LATCH_8_ADD("soundlatch")
	MCFG_GENERIC_LATCH_DATA_PENDING_CB(INPUTLINE("audiocpu", 0))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(mouser_state, screen_update_mouser)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_VBLANK_CALLBACK(WRITELINE(mouser_state, mouser_nmi_interrupt))

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mouser)
	MCFG_PALETTE_ADD("palette", 64)
	MCFG_PALETTE_INIT_OWNER(mouser_state, mouser)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 4000000/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("ay2", AY8910, 4000000/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


ROM_START( mouser )
	ROM_REGION( 0x6000, "maincpu", 0 ) /* 64K for data, 64K for encrypted opcodes */
	ROM_LOAD( "m0.5e",         0x0000, 0x2000, CRC(b56e00bc) SHA1(f3b23212590d91f1d19b1c7a98c560fbe5943185) )
	ROM_LOAD( "m1.5f",         0x2000, 0x2000, CRC(ae375d49) SHA1(8422f5a4d8560425f0c8612cf6f76029fcfe267c) )
	ROM_LOAD( "m2.5j",         0x4000, 0x2000, CRC(ef5817e4) SHA1(5cadc19f20fadf97c95852b280305fe4c75f1d19) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "m5.3v",         0x0000, 0x1000, CRC(50705eec) SHA1(252cea3498722318638f0c98ae929463ffd7d0d6) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "m3.11h",        0x0000, 0x2000, CRC(aca2834e) SHA1(c4f457fd8ea46386431ef8dffe54a232631870be) )
	ROM_LOAD( "m4.11k",        0x2000, 0x2000, CRC(943ab2e2) SHA1(ef9fc31dc8fe7a62f7bc6c817ce0d65091cb9a03) )

	/* Opcode Decryption PROMS */
	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD_NIB_HIGH( "bprom.4b",0x0000,0x0100,CRC(dd233851) SHA1(25eab1ec2227910c6fcd2803986f1cf206624da7) )
	ROM_LOAD_NIB_LOW(  "bprom.4c",0x0000,0x0100,CRC(60aaa686) SHA1(bb2ad555da51f6b30ab8b55833fe8d461a1e67f4) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "bprom.5v", 0x0000, 0x0020, CRC(7f8930b2) SHA1(8d0fe14b770fcf7088696d7b80d64507c6ee7364) )
	ROM_LOAD( "bprom.5u", 0x0020, 0x0020, CRC(0086feed) SHA1(b0b368e5fb7380cf09abd60c0933b405daf1c36a) )
ROM_END


ROM_START( mouserc )
	ROM_REGION( 0x6000, "maincpu", 0 ) /* 64K for data, 64K for encrypted opcodes */
	ROM_LOAD( "83001.0",       0x0000, 0x2000, CRC(e20f9601) SHA1(f559a470784bda0bee9cab257a548238365acaa6) )
	ROM_LOAD( "m1.5f",         0x2000, 0x2000, CRC(ae375d49) SHA1(8422f5a4d8560425f0c8612cf6f76029fcfe267c) )   // 83001.1
	ROM_LOAD( "m2.5j",         0x4000, 0x2000, CRC(ef5817e4) SHA1(5cadc19f20fadf97c95852b280305fe4c75f1d19) )   // 83001.2

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "m5.3v",         0x0000, 0x1000, CRC(50705eec) SHA1(252cea3498722318638f0c98ae929463ffd7d0d6) )   // 83001.5

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "m3.11h",        0x0000, 0x2000, CRC(aca2834e) SHA1(c4f457fd8ea46386431ef8dffe54a232631870be) )   // 83001.3
	ROM_LOAD( "m4.11k",        0x2000, 0x2000, CRC(943ab2e2) SHA1(ef9fc31dc8fe7a62f7bc6c817ce0d65091cb9a03) )   // 83001.4

	/* Opcode Decryption PROMS (originally from the UPL romset!) */
	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD_NIB_HIGH( "bprom.4b",0x0000,0x0100,CRC(dd233851) SHA1(25eab1ec2227910c6fcd2803986f1cf206624da7) )
	ROM_LOAD_NIB_LOW(  "bprom.4c",0x0000,0x0100,CRC(60aaa686) SHA1(bb2ad555da51f6b30ab8b55833fe8d461a1e67f4) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "bprom.5v", 0x0000, 0x0020, CRC(7f8930b2) SHA1(8d0fe14b770fcf7088696d7b80d64507c6ee7364) )    // clr.5v
	ROM_LOAD( "bprom.5u", 0x0020, 0x0020, CRC(0086feed) SHA1(b0b368e5fb7380cf09abd60c0933b405daf1c36a) )    // clr.5u
ROM_END


DRIVER_INIT_MEMBER(mouser_state,mouser)
{
	/* Decode the opcodes */

	offs_t i;
	uint8_t *rom = memregion("maincpu")->base();
	uint8_t *table = memregion("user1")->base();

	for (i = 0; i < 0x6000; i++)
	{
		m_decrypted_opcodes[i] = table[rom[i]];
	}
}


GAME( 1983, mouser,   0,      mouser, mouser, mouser_state, mouser, ROT90, "UPL",                  "Mouser",          MACHINE_SUPPORTS_SAVE )
GAME( 1983, mouserc,  mouser, mouser, mouser, mouser_state, mouser, ROT90, "UPL (Cosmos license)", "Mouser (Cosmos)", MACHINE_SUPPORTS_SAVE )
