#ifndef FLAGSTATUS_HPP
#define FLAGSTATUS_HPP

#include "KeyCode.hpp"
#include "bridge.h"

namespace org_pqrs_KeyRemap4MacBook {
  class FlagStatus {
  public:
    class Item {
      friend class FlagStatus;

    public:
      Item(void) :
        flag_(ModifierFlag::ZERO),
        count_(0),
        temporary_count_(0),
        lock_count_(0),
        sticky_count_(0)
      {}

    private:
      void initialize(ModifierFlag f);
      void set(void);
      void set(KeyCode key, Flags flags);

      void reset(void);
      int sum(void) const { return count_ + temporary_count_ + lock_count_ + sticky_count_; }

      void increase(void);
      void decrease(void);
      void temporary_increase(void) { ++temporary_count_; }
      void temporary_decrease(void) { --temporary_count_; }
      void lock_increase(void) { lock_count_ = 1; }
      void lock_decrease(void) { lock_count_ = 0; }
      void lock_toggle(void)   { lock_count_ = ! lock_count_; }
      void sticky_increase(void) { sticky_count_ = 1; }
      void sticky_decrease(void) { sticky_count_ = 0; }
      void sticky_toggle(void) { sticky_count_ = ! sticky_count_; }

      ModifierFlag flag_;
      int count_;
      int temporary_count_;

      int lock_count_; // store remapped lock status. (CapsLock, FN lock, ...)

      int sticky_count_;
    };
    DECLARE_VECTOR(Item);

    class ScopedTemporaryFlagsChanger {
    public:
      ScopedTemporaryFlagsChanger(FlagStatus& flagStatus, Flags toFlags) : flagStatus_(flagStatus) {
        for (size_t i = 0; i < flagStatus_.item_.size(); ++i) {
          count_.push_back(0);

          // ----------------------------------------
          // reset flag
          while (flagStatus_.item_[i].sum() < 0) {
            flagStatus_.item_[i].temporary_increase();
            ++(count_.back());
          }
          while (flagStatus_.item_[i].sum() > 0) {
            flagStatus_.item_[i].temporary_decrease();
            --(count_.back());
          }

          // ----------------------------------------
          // set a flag
          ModifierFlag flag = flagStatus_.getFlag(i);
          if (toFlags.isOn(flag)) {
            flagStatus_.item_[i].temporary_increase();
            ++(count_.back());
          }
        }
      }
      ~ScopedTemporaryFlagsChanger(void) {
        for (size_t i = 0; i < flagStatus_.item_.size(); ++i) {
          if (i >= count_.size()) break;

          while (count_[i] < 0) {
            flagStatus_.item_[i].temporary_increase();
            ++(count_[i]);
          }
          while (count_[i] > 0) {
            flagStatus_.item_[i].temporary_decrease();
            --(count_[i]);
          }
        }
      }

    private:
      FlagStatus& flagStatus_;
      Vector_int count_;
    };

    FlagStatus(void);
    FlagStatus(Flags flags);

    void set(void);
    void set(KeyCode key, Flags flags);
    Flags makeFlags(void) const;
    // get registered ModifierFlag by index.
    ModifierFlag getFlag(size_t index) const;
    void reset(void);

    size_t itemSize(void) const { return item_.size(); }

    bool isOn(const Vector_ModifierFlag& modifierFlags) const;

    Flags getLockedFlags(void) const;
    Flags getStickyFlags(void) const;

#define DECLARE_METHODS(METHOD)                                                     \
  void METHOD(ModifierFlag modifierFlag);                                           \
  void METHOD(ModifierFlag modifierFlag, const Vector_ModifierFlag &modifierFlags); \
  void METHOD(const Vector_ModifierFlag &modifierFlags);                            \

    DECLARE_METHODS(increase)
    DECLARE_METHODS(decrease)
    DECLARE_METHODS(temporary_increase)
    DECLARE_METHODS(temporary_decrease)
    DECLARE_METHODS(lock_increase)
    DECLARE_METHODS(lock_decrease)
    DECLARE_METHODS(lock_toggle)
    DECLARE_METHODS(sticky_increase)
    DECLARE_METHODS(sticky_decrease)
    DECLARE_METHODS(sticky_toggle)
#undef DECLARE_METHODS

    void sticky_clear(void);
    void lock_clear(void);

    static FlagStatus& globalFlagStatus(void);

  private:
    void initialize(void);
    bool isOn(ModifierFlag modifierFlag) const;
    void updateStatusMessage(void);

    Flags statusMessageFlags_[BRIDGE_USERCLIENT_STATUS_MESSAGE__END__];
    Vector_Item item_;
  };
}

#endif
