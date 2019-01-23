// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTAIController.h"

void ASDTAIController::Tick(float deltaTime)
{
	Move(m_maxAcceleration, m_maxSpeed);
}

//Ralentir avant obstacle


void ASDTAIController::Move(float acceleration, float vitesse) {
	APawn* pawn = GetPawn();
	FVector direction = pawn->GetActorForwardVector();
	
	pawn->AddMovementInput(direction, vitesse);// Il y a un booléen qu'on peut utiliser dans def de la fonctionbool bForce = false;

}





