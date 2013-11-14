from ert.job_queue import JobStatusType
from ert_gui.models.connectors.run.run_status import RunStatusModel
from ert_gui.models.mixins import ListModelMixin


class SimulationStateStatus(object):
    WAITING = (164, 164, 255)
    PENDING = (164, 200, 255)
    RUNNING = (200, 255, 200)
    FAILED  = (255, 200, 200)

    USER_KILLED = (255, 255, 200)
    FINISHED   = (200, 200, 200)
    NOT_ACTIVE  = (255, 255, 255)

    def __init__(self, name, state, color):
        self.__name = name
        self.__state = state
        self.__color = color

        self.__count = 0
        self.__total_count = 1

    @property
    def name(self):
        return self.__name

    @property
    def state(self):
        return self.__state

    @property
    def color(self):
        return self.__color

    @property
    def count(self):
        return self.__count

    @count.setter
    def count(self, value):
        self.__count = value

    @property
    def total_count(self):
        return self.__total_count

    @total_count.setter
    def total_count(self, value):
        self.__total_count = value


class SimulationsTracker(ListModelMixin):
    def __init__(self):
        super(SimulationsTracker, self).__init__()
        RunStatusModel().observable().attach(RunStatusModel.STATUS_CHANGED_EVENT, self.__statusChanged)

        waiting_state = SimulationStateStatus("Waiting", JobStatusType.JOB_QUEUE_NOT_ACTIVE | JobStatusType.JOB_QUEUE_WAITING | JobStatusType.JOB_QUEUE_SUBMITTED, SimulationStateStatus.WAITING)
        pending_state = SimulationStateStatus("Pending", JobStatusType.JOB_QUEUE_PENDING, SimulationStateStatus.PENDING)
        running_state = SimulationStateStatus("Running", JobStatusType.JOB_QUEUE_RUNNING | JobStatusType.JOB_QUEUE_EXIT | JobStatusType.JOB_QUEUE_RUNNING_CALLBACK, SimulationStateStatus.RUNNING)
        killed_state = SimulationStateStatus("User killed", JobStatusType.JOB_QUEUE_USER_KILLED | JobStatusType.JOB_QUEUE_USER_EXIT, SimulationStateStatus.USER_KILLED)
        failed_state = SimulationStateStatus("Failed", JobStatusType.JOB_QUEUE_FAILED, SimulationStateStatus.FAILED)
        done_state = SimulationStateStatus("Finished", JobStatusType.JOB_QUEUE_DONE | JobStatusType.JOB_QUEUE_SUCCESS, SimulationStateStatus.FINISHED)

        self.states = [waiting_state, pending_state, running_state, killed_state, failed_state, done_state]
        self.__done_state = done_state
        self.__progress = 0

        self.__checkForUnusedEnums()

    def __statusChanged(self):
        status_counts = RunStatusModel().getStatusCounts()
        total_count = RunStatusModel().getActiveCount()

        for state in self.states:
            state.count = 0
            state.total_count = total_count

        for status in status_counts:
            count = status_counts[status]

            for state in self.states:
                if status in state.state:
                    state.count += count

        self.__progress = float(self.__done_state.count) / total_count


        self.observable().notify(ListModelMixin.LIST_CHANGED_EVENT)


    def getList(self):
        """ @rtype: list of SimulationStateStatus """
        return list(self.states)

    def getProgress(self):
        return  self.__progress


    def __checkForUnusedEnums(self):
        for enum in JobStatusType.enums():
            used = False
            for state in self.states:
                if enum in state.state:
                    used = True

            if not used:
                raise AssertionError("Enum identifier '%s' not used!" % enum)




