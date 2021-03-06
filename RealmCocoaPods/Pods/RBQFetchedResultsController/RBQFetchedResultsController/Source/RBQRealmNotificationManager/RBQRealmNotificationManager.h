//
//  RBQRealmNotificationManager.h
//  RBQRealmNotificationManage
//
//  Created by Adam Fish on 1/4/15.
//  Copyright (c) 2015 Roobiq. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <Realm/Realm.h>

@class RBQSafeRealmObject;

#pragma mark - RBQClassChangesObject

/**
 *  Class used by the RBQRealmNotificationManager to represent the change set for a given entity. This object is passed in the NSDictionary (keyed by the entity name) contained in the RBQNotificationBlock after a change to monitored Realm.
 */
@interface RBQEntityChangesObject : NSObject

/**
 *  The class name of the entity
 */
@property (readonly, nonatomic, nonnull) NSString *className;

/**
 *  Collection of RBQSafeRealmObjects representing the added objects
 */
@property (readonly, nonatomic, nonnull) NSSet *addedSafeObjects;

/**
 *  Collection of RBQSafeRealmObjects representing the deleted objects
 */
@property (readonly, nonatomic, nonnull) NSSet *deletedSafeObjects;

/**
 *  Collection of RBQSafeRealmObjects representing the changed objects
 */
@property (readonly, nonatomic, nonnull) NSSet *changedSafeObjects;

+ (nonnull instancetype)createEntityChangeObjectWithClassName:(nonnull NSString *)className;

- (void)didAddSafeObject:(nonnull RBQSafeRealmObject *)safeObject;

- (void)willDeleteSafeObject:(nonnull RBQSafeRealmObject *)safeObject;

- (void)didChangeSafeObject:(nonnull RBQSafeRealmObject *)safeObject;

@end

#pragma mark - Constants

/**
 *  When added to a RBQRealmNotificationManager, this block fires when the tracked Realm changes.
 *
 *  @param entityChanges NSDictionary with the keys represented as the class name of an entity that had changes. The object in the dictionary is a RBQEntityChangesObject, which contains the specific changes.
 *  @param realm         RLMRealm that updated (this is the original RLMRealm instance that was acted on to perform the changes. Not thread-safe).
 */
NS_ASSUME_NONNULL_BEGIN
typedef void(^RBQNotificationBlock)(NSDictionary *entityChanges,
                                    RLMRealm *realm);
NS_ASSUME_NONNULL_END

@interface RBQNotificationToken : NSObject

@end

/**
 *  This class is used to track changes to a given RLMRealm. Since Realm doesn't support automatic change tracking, this class allows the developer to log object changes, which will be passed along to the RBQRealmNotificationManager who in turn broadcasts it to any listeners
 *
 *  Since RLMObjects are not thread-safe, when an object is logged to the manager, it is internally transformed into an RBQSafeRealmObject that is thread-safe and this will then be passed to any listeners once the Realm being monitored updates.
 *
 *  @warning Only RLMObjects with primary keys can be logged because the primary key is required to create a RBQSafeRealmObject.
 */
@interface RBQRealmChangeLogger : NSObject

@property (readonly, nonatomic, nonnull) NSDictionary *entityChanges;

/**
 *  Creates or retrieves the logger instance for the default Realm on the current thread
 *
 *  @return Instance of RBQRealmChangeLogger
 */
+ (nonnull instancetype)defaultLogger;

/**
 *  Creates or retrieves the logger instance for a specific Realm on the current thread
 *
 *  @param realm A RLMRealm instance
 *
 *  @return Instance of RBQRealmChangeLogger
 */
+ (nonnull instancetype)loggerForRealm:(nonnull RLMRealm *)realm;

/**
 *  Register an addition for a given RLMObject
 *
 *  @warning Can be called before or after the addition to Realm
 *
 *  @param addedObject Added RLMObject
 */
- (void)didAddObject:(nonnull RLMObjectBase *)addedObject;

/**
 *  Register a collection of RLMObject additions
 *
 *  @warning Can be called before or after the additions to Realm
 *
 *  @param addedObjects RLMArray, RLMResults, NSSet, or NSArray of added RLMObjects
 */
- (void)didAddObjects:(nonnull id<NSFastEnumeration>)addedObjects;

/**
 *  Register a delete for a given RLMObject
 *
 *  @warning Must be called before the delete in Realm (since the RLMObject will then be invalidated).
 *
 *  @param deletedObject To be deleted RLMObject
 */
- (void)willDeleteObject:(nonnull RLMObjectBase *)deletedObject;

/**
 *  Register a collection of RLMObject deletes
 *
 *  @warning Must be called before the delete in Realm (since the RLMObject will then be invalidated).
 *
 *  @param deletedObjects RLMArray, RLMResults, NSSet, or NSArray of deleted RLMObjects
 */
- (void)willDeleteObjects:(nonnull id<NSFastEnumeration>)deletedObjects;

/**
 *  Register a change for a given RLMObject
 *
 *  @warning Can be called before or after change to Realm
 *
 *  @param changedObject Changed RLMObject
 */
- (void)didChangeObject:(nonnull RLMObjectBase *)changedObject;

/**
 *  Register a collection of RLMObject changes
 *
 *  @warning Can be called before or after change to Realm
 *
 *  @param changedObjects RLMArray, RLMResults, NSSet, or NSArray of changed RLMObjects
 */
- (void)didChangeObjects:(nonnull id<NSFastEnumeration>)changedObjects;

/**
 *  Convenience method to pass array of objects changed. Will ignore nil values;
 *
 *  @param addedObjects   RLMArray, RLMResults, NSSet, or NSArray of added RLMObjects
 *  @param deletedObjects RLMArray, RLMResults, NSSet, or NSArray of deleted RLMObjects
 *  @param changedObjects RLMArray, RLMResults, NSSet, or NSArray of changed RLMObjects
 */
- (void)didAddObjects:(nonnull id<NSFastEnumeration>)addedObjects
    willDeleteObjects:(nonnull id<NSFastEnumeration>)deletedObjects
     didChangeObjects:(nonnull id<NSFastEnumeration>)changedObjects;

@end

/**
 *  This class works in conjunction with any instances of RBQRealmChangeLogger to broadcast any changes to the registered listeners
 */
@interface RBQRealmNotificationManager : NSObject

/**
 *  Retrieve the singleton RBQRealmNotificationManager that passes changes from all Realm loggers
 *
 *  @return Singleton RBQRealmNotificationManager
 */
+ (nonnull instancetype)defaultManager;

/**
 *  Use this method to add a notification block that will fire every time the Realm for this RBQNotificationManager updates. The block passes the changes from the Realm update that were logged to the RBQRealmNotificationManager.
 *
 *  @param block RBQNotificationBlock that passes a NSDictionary keyed by entity name. The object for the key is a RBQEntityChangesObject which contains NSSets of all the various changes to the entity.
 *
 *  @warning You must hold onto a strong reference to the returned token or it will be deallocated, preventing any changes from propogating.
 *
 *  @see RBQEntityChangesObject
 *  @see RBQNotificationBlock
 *
 *  @return A new instance of RBQNotificationToken.
 */
- (nonnull RBQNotificationToken *)addNotificationBlock:(nonnull RBQNotificationBlock)block;

/**
 *  De-register a notification given a RBQNotificationToken.
 *
 *  @param token The RBQNotificationToken to be de-registered.
 */
- (void)removeNotification:(nonnull RBQNotificationToken *)token;

@end
