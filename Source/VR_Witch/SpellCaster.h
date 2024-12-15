// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpellCaster.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VR_WITCH_API USpellCaster : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USpellCaster();
	TArray<TArray<FVector2D>> templates;
	UFUNCTION(BlueprintCallable, Category = "Spell Casting")
	void Recognize(TArray<FVector2D> points, double& score, int& templateIndex);
	UFUNCTION(BlueprintCallable, Category = "Spell Casting")
	void AddTemplate(TArray<FVector2D> points);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	TArray<FVector2D> Resample(TArray<FVector2D> points,int resampledPointAmount);
	double IndicativeAngle(TArray<FVector2D> points);
	TArray<FVector2D> Rotate(TArray<FVector2D> points, double radians);
	TArray<FVector2D> Scale(TArray<FVector2D> points, double newSize);
	TArray<FVector2D> Translate(TArray<FVector2D> points, FVector2D toPoint);


	FVector2D FindCentroid(TArray<FVector2D> points);
	double DistanceAtAngle(TArray<FVector2D> samples, TArray<FVector2D> theTemplate, float theta);
	double PathDistance(TArray<FVector2D> points1, TArray<FVector2D> points2);
	double DistanceAtBestAngle(TArray<FVector2D> points, TArray<FVector2D> streokTemplate, double fromAngle, double toAngle, double threshold);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
