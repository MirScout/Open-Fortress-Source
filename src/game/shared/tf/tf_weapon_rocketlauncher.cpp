//====== Copyright � 1996-2005, Valve Corporation, All rights reserved. =======
//
// TF Rocket Launcher
//
//=============================================================================
#include "cbase.h"
#include "tf_weapon_rocketlauncher.h"
#include "tf_fx_shared.h"
#include "tf_weaponbase_rocket.h"
#include "in_buttons.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
// Server specific.
#else
#include "tf_player.h"
#endif

//=============================================================================
//
// Weapon Rocket Launcher tables.
//
IMPLEMENT_NETWORKCLASS_ALIASED( TFRocketLauncher, DT_WeaponRocketLauncher )

BEGIN_NETWORK_TABLE( CTFRocketLauncher, DT_WeaponRocketLauncher )
#ifndef CLIENT_DLL
	//SendPropBool( SENDINFO(m_bLockedOn) ),
#else
	//RecvPropInt( RECVINFO(m_bLockedOn) ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFRocketLauncher )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_rocketlauncher, CTFRocketLauncher );
PRECACHE_WEAPON_REGISTER( tf_weapon_rocketlauncher );

// Server specific.
#ifndef CLIENT_DLL
BEGIN_DATADESC( CTFRocketLauncher )
END_DATADESC()
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( TFCRPG, DT_TFCRPG )

BEGIN_NETWORK_TABLE( CTFCRPG, DT_TFCRPG )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFCRPG)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tfc_weapon_rpg, CTFCRPG );
PRECACHE_WEAPON_REGISTER( tfc_weapon_rpg );

// Server specific.
#ifndef CLIENT_DLL
BEGIN_DATADESC( CTFCRPG )
END_DATADESC()
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( TFCIncendiaryCannon, DT_TFCIncendiaryCannon )

BEGIN_NETWORK_TABLE( CTFCIncendiaryCannon, DT_TFCIncendiaryCannon )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFCIncendiaryCannon)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tfc_weapon_incendiarycannon, CTFCIncendiaryCannon );
PRECACHE_WEAPON_REGISTER( tfc_weapon_incendiarycannon );

// Server specific.
#ifndef CLIENT_DLL
BEGIN_DATADESC( CTFCIncendiaryCannon )
END_DATADESC()
#endif

ConVar ofd_quad_explode_delay( "ofd_quad_explode_delay", "0.2", FCVAR_REPLICATED, "How long the rocket has to be active for before you can detonate it." );

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
//-----------------------------------------------------------------------------
CTFRocketLauncher::CTFRocketLauncher()
{
	m_bReloadsSingly = true;
}

CTFSuperRocketLauncher::CTFSuperRocketLauncher()
{
#ifdef CLIENT_DLL
	m_pEffect = NULL;
#endif
	m_bReloadsSingly = false;
	m_flLastPingSoundTime = 0;
}

CTFCRPG::CTFCRPG()
{
	m_bReloadsSingly = true;
}

CTFCIncendiaryCannon::CTFCIncendiaryCannon()
{
	m_bReloadsSingly = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
//-----------------------------------------------------------------------------
CTFRocketLauncher::~CTFRocketLauncher()
{
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFRocketLauncher::Precache()
{
	BaseClass::Precache();
	PrecacheParticleSystem( "rocketbackblast" );
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseEntity *CTFRocketLauncher::FireProjectile( CTFPlayer *pPlayer )
{
	m_flShowReloadHintAt = gpGlobals->curtime + 30;
	CBaseEntity *pProjectile = BaseClass::FireProjectile( pPlayer );
	if ( pProjectile )
	{
#ifdef GAME_DLL
		CTFBaseRocket *pRocket = (CTFBaseRocket*)pProjectile;
		pRocket->SetLauncher( this );

		RocketHandle hHandle;
		hHandle = pRocket;
		m_Rockets.AddToTail( hHandle );

		m_iRocketCount = m_Rockets.Count();
 #endif
	}

	return pProjectile;	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFRocketLauncher::ItemPostFrame( void )
{
	CTFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
	if ( !pOwner )
		return;

	BaseClass::ItemPostFrame();

#ifdef GAME_DLL

	if ( m_flShowReloadHintAt && m_flShowReloadHintAt < gpGlobals->curtime )
	{
		if ( Clip1() < GetMaxClip1() )
		{
			pOwner->HintMessage( HINT_SOLDIER_RPG_RELOAD );
		}
		m_flShowReloadHintAt = 0;
	}

	/*
	Vector forward;
	AngleVectors( pOwner->EyeAngles(), &forward );
	trace_t tr;
	CTraceFilterSimple filter( pOwner, COLLISION_GROUP_NONE );
	UTIL_TraceLine( pOwner->EyePosition(), pOwner->EyePosition() + forward * 2000, MASK_SOLID, &filter, &tr );

	if ( tr.m_pEnt &&
		tr.m_pEnt->IsPlayer() &&
		tr.m_pEnt->IsAlive() &&
		tr.m_pEnt->GetTeamNumber() != pOwner->GetTeamNumber() )
	{
		m_bLockedOn = true;
	}
	else
	{
		m_bLockedOn = false;
	}
	*/

#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFRocketLauncher::Deploy( void )
{
	if ( BaseClass::Deploy() )
	{
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFRocketLauncher::DefaultReload( int iClipSize1, int iClipSize2, int iActivity )
{
	m_flShowReloadHintAt = 0;
	return BaseClass::DefaultReload( iClipSize1, iClipSize2, iActivity );
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFRocketLauncher::CreateMuzzleFlashEffects( C_BaseEntity *pAttachEnt, int nIndex )
{
	BaseClass::CreateMuzzleFlashEffects( pAttachEnt, nIndex );

	// Don't do backblast effects in first person
	C_TFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
	if ( pOwner->IsLocalPlayer() )
		return;

	ParticleProp()->Create( "rocketbackblast", PATTACH_POINT_FOLLOW, "backblast" );
}

/*
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFRocketLauncher::DrawCrosshair( void )
{
	BaseClass::DrawCrosshair();

	if ( m_bLockedOn )
	{
		int iXpos = XRES(340);
		int iYpos = YRES(260);
		int iWide = XRES(8);
		int iTall = YRES(8);

		Color col( 0, 255, 0, 255 );
		vgui::surface()->DrawSetColor( col );

		vgui::surface()->DrawFilledRect( iXpos, iYpos, iXpos + iWide, iYpos + iTall );

		// Draw the charge level onscreen
		vgui::HScheme scheme = vgui::scheme()->GetScheme( "ClientScheme" );
		vgui::HFont hFont = vgui::scheme()->GetIScheme(scheme)->GetFont( "Default" );
		vgui::surface()->DrawSetTextFont( hFont );
		vgui::surface()->DrawSetTextColor( col );
		vgui::surface()->DrawSetTextPos(iXpos + XRES(12), iYpos );
		vgui::surface()->DrawPrintText(L"Lock", wcslen(L"Lock"));

		vgui::surface()->DrawLine( XRES(320), YRES(240), iXpos, iYpos );
	}
}
*/

#endif


void CTFSuperRocketLauncher::AddRocket( CTFBaseRocket *pRocket )
{
	RocketHandle hHandle;
	hHandle = pRocket;
	m_Rockets.AddToTail( hHandle );
}


//-----------------------------------------------------------------------------
// Purpose: Detonate the Rockets if secondary fire is down.
//-----------------------------------------------------------------------------
void CTFSuperRocketLauncher::ItemPostFrame( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner && pOwner->m_nButtons & IN_ATTACK2 )
	{
		// We need to do this to catch the case of player trying to detonate
		// pipebombs while in the middle of reloading.
		SecondaryAttack();
	}
#ifdef CLIENT_DLL
	else if ( m_flLastPingSoundTime <= gpGlobals->curtime )
	{
		if ( m_pEffect )
		{
			ParticleProp()->StopEmission( m_pEffect );
			m_pEffect = NULL;
		}
	}
#endif
	BaseClass::ItemPostFrame();
}

//-----------------------------------------------------------------------------
// Purpose: Detonate active pipebombs
//-----------------------------------------------------------------------------
void CTFSuperRocketLauncher::SecondaryAttack( void )
{
	if ( !CanAttack() || !CanSecondaryAttack() )
		return;

	if ( m_iRocketCount )
	{
		// Get a valid player.
		CTFPlayer *pPlayer = ToTFPlayer( GetOwner() );
		if ( !pPlayer )
			return;
		//If one or more pipebombs failed to detonate then play a sound.
		if ( DetonateRockets() && m_flLastPingSoundTime <= gpGlobals->curtime )
		{
			// PING!
#ifdef CLIENT_DLL
			C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
			if ( pLocalPlayer && pLocalPlayer == GetOwner() )
			{
				if ( pLocalPlayer->GetViewModel() )
				{
					if ( m_pEffect )
					{
						ParticleProp()->StopEmission( m_pEffect );
						m_pEffect = NULL;
					}

					if ( !m_pEffect )
						m_pEffect = pLocalPlayer->GetViewModel()->ParticleProp()->Create( "quad_ping", PATTACH_POINT_FOLLOW, "ping" );
				}
			}
#endif
			m_flLastPingSoundTime = gpGlobals->curtime + 1;
			WeaponSound( SPECIAL3 );
		}	
	}
}

//-----------------------------------------------------------------------------
// Purpose: If a pipebomb has been removed, remove it from our list
//-----------------------------------------------------------------------------
void CTFSuperRocketLauncher::DeathNotice( CBaseEntity *pVictim )
{
	Assert( dynamic_cast<CTFBaseRocket*>(pVictim) );

	RocketHandle hHandle;
	hHandle = (CTFBaseRocket*)pVictim;
	m_Rockets.FindAndRemove( hHandle );

	m_iRocketCount = m_Rockets.Count();
}

//-----------------------------------------------------------------------------
// Purpose: Remove *with* explosions
//-----------------------------------------------------------------------------
bool CTFSuperRocketLauncher::DetonateRockets()
{

	int count = m_Rockets.Count();

	bool bDetonated = true;
	
	for ( int i = 0; i < count; i++ )
	{
		CTFBaseRocket *pTemp = m_Rockets[i];
		if ( pTemp )
		{
			//This guy will die soon enough.
			if ( pTemp->IsEffectActive( EF_NODRAW ) || pTemp->m_flCreationTime + ofd_quad_explode_delay.GetFloat() > gpGlobals->curtime )
				continue;
#ifdef GAME_DLL
			pTemp->Detonate();
#endif
			bDetonated = true;
		}
	}
	return bDetonated;
}

IMPLEMENT_NETWORKCLASS_ALIASED(TFOriginal, DT_TFOriginal);

BEGIN_NETWORK_TABLE( CTFOriginal, DT_TFOriginal )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFOriginal )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(tf_weapon_rocketlauncher_dm, CTFOriginal);
PRECACHE_WEAPON_REGISTER(tf_weapon_rocketlauncher_dm);

IMPLEMENT_NETWORKCLASS_ALIASED(TFSuperRocketLauncher, DT_TFSuperRocketLauncher);

BEGIN_NETWORK_TABLE(CTFSuperRocketLauncher, DT_TFSuperRocketLauncher)
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_iRocketCount ) ),
#else
	SendPropInt( SENDINFO( m_iRocketCount ), 5, SPROP_UNSIGNED ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFSuperRocketLauncher )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(tf_weapon_super_rocketlauncher, CTFSuperRocketLauncher);
PRECACHE_WEAPON_REGISTER(tf_weapon_super_rocketlauncher);