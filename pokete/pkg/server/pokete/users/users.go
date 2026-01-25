package users

import (
	"errors"
	"fmt"
	"log/slog"

	"github.com/lxgr-linux/pokete/server/pokete/positions"
	"github.com/lxgr-linux/pokete/server/pokete/user"
)

var (
	USER_PRESENT      error = errors.New("newUser already present")
	USER_DOESNT_EXIST error = errors.New("user doesn't exist")
)

type Users struct {
	users     *map[uint64]user.User
	nameMap   *map[string]uint64
	positions *positions.Positions
}

func (u Users) Add(conId uint64, newUser user.User) error {
	if _, ok := (*u.nameMap)[newUser.Name]; ok {
		return USER_PRESENT
	}

	(*u.users)[conId] = newUser
	(*u.nameMap)[newUser.Name] = conId
	err := u.positions.BroadcastChange(conId, newUser)

	return err
}

func (u Users) Remove(conId uint64) {
	us, ok := (*u.users)[conId]
	if ok {
		_ = u.positions.BroadcastRemoval(conId, us.Name)
		delete(*u.nameMap, us.Name)
	}
	u.positions.UnSubscribe(conId)
	delete(*u.users, conId)
}

func (u Users) GetAllUsers() (retUsers []user.User) {
	for _, us := range *u.users {
		retUsers = append(retUsers, us)
	}
	return
}

func (u Users) GetUserByName(name string) (*user.User, error) {
	conId, ok := (*u.nameMap)[name]
	if !ok {
		return nil, USER_DOESNT_EXIST
	}
	us, ok := (*u.users)[conId]
	if !ok {
		return nil, USER_DOESNT_EXIST
	}
	return &us, nil
}

func (u Users) GetUserByConId(conId uint64) (*user.User, error) {
	us, ok := (*u.users)[conId]
	if !ok {
		return nil, USER_DOESNT_EXIST
	}
	return &us, nil
}

func (u Users) GetAllUserNames() (names []string) {
	for _, us := range *u.users {
		names = append(names, us.Name)
	}
	return
}

func (u Users) SetNewPositionToUser(conId uint64, newPosition user.Position) error {
	us := (*u.users)[conId]
	err := us.Position.Change(newPosition)
	(*u.users)[conId] = us
	go func() {
		err := u.positions.BroadcastChange(conId, us)
		if err != nil {
			slog.Error(fmt.Sprintf("Error broadcasting position update: %s", err))
		}
	}()
	return err
}

func NewUsers(positions2 *positions.Positions) *Users {
	var tempUsers = make(map[uint64]user.User)
	var tempNameMap = make(map[string]uint64)
	return &Users{
		users:     &tempUsers,
		nameMap:   &tempNameMap,
		positions: positions2,
	}
}
