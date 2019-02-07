// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTCollectible.h"

ASDTCollectible::ASDTCollectible()
{
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleAsset(TEXT("ParticleSystem'/Game/StarterContent/Particles/P_Explosion.P_Explosion'"));

	fx = ParticleAsset.Object;

	static ConstructorHelpers::FObjectFinder<USoundBase> Soundf(TEXT("SoundWave'/Game/StarterContent/Audio/Explosion01.Explosion01'"));

	sound = Soundf.Object;
}

//void ASDTCollectible::PostInitializeComponents()
//{
//	Super::PostInitializeComponents();
//}

void ASDTCollectible::Collect()
{
	if (!IsOnCooldown())
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), fx, GetActorLocation(), FRotator::ZeroRotator, true);

		UGameplayStatics::PlaySound2D(GetWorld(), sound);
	}

    GetWorld()->GetTimerManager().SetTimer(m_CollectCooldownTimer, this, &ASDTCollectible::OnCooldownDone, m_CollectCooldownDuration, false);

    GetStaticMeshComponent()->SetVisibility(false);
}

void ASDTCollectible::OnCooldownDone()
{
    GetWorld()->GetTimerManager().ClearTimer(m_CollectCooldownTimer);

    GetStaticMeshComponent()->SetVisibility(true);
}

bool ASDTCollectible::IsOnCooldown()
{
    return m_CollectCooldownTimer.IsValid();
}
